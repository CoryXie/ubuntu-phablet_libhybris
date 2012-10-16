#include "application_manager.h"
#include "default_application_manager_input_setup.h"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>

#include <input/InputListener.h>
#include <input/InputReader.h>
#include <ui/InputTransport.h>
#include <utils/threads.h>

#include <cstdio>

namespace
{

struct ApplicationManager : public android::BnApplicationManager,
                            public android::IBinder::DeathRecipient
{
    static const int shell_components_base_layer = 200;
    static const int focused_application_base_layer = 100;
    static const int wallpaper_layer = 0;
    static const int non_focused_application_layer = -1;

    struct ApplicationSession : public android::RefBase
    {
        ApplicationSession(
            android::sp<android::IApplicationManagerSession> remote_session,
            const android::String8& app_name,
            const android::sp<android::InputChannel>& input_channel) 
                : remote_session(remote_session),
                  app_name(app_name),
                  input_channel(input_channel),
                  input_publisher(input_channel)
        {
            input_publisher.initialize();
        }

        void dispatch_key_event(const android::NotifyKeyArgs& args)
        {
            input_publisher.publishKeyEvent(
                args.deviceId,
                args.source,
                args.action,
                args.flags,
                args.keyCode,
                args.scanCode,
                args.metaState,
                0, // repeatCount
                args.downTime,
                args.eventTime);

            input_publisher.sendDispatchSignal();
        }

        void dispatch_motion_event(const android::NotifyMotionArgs& args)
        {
            input_publisher.publishMotionEvent(
                args.deviceId,
                args.source,
                args.action,
                args.flags,
                args.edgeFlags,
                args.metaState,
                args.buttonState,
                0.f, //args.xOffset,
                0.f, // args.yOffset,
                args.xPrecision,
                args.yPrecision,
                args.downTime,
                args.eventTime,
                args.pointerCount,
                args.pointerProperties,
                args.pointerCoords);

            input_publisher.sendDispatchSignal();
        }

        android::sp<android::IApplicationManagerSession> remote_session;
        android::String8 app_name;
        android::sp<android::InputChannel> input_channel;
        android::InputPublisher input_publisher;
    };

    class InputListener : public android::InputListenerInterface
    {
      public:
        InputListener(ApplicationManager* manager) : manager(manager)
        {
        }

        void notifyConfigurationChanged(const android::NotifyConfigurationChangedArgs* args)
        {
            (void) args;
        }

        void notifyKey(const android::NotifyKeyArgs* args)
        {
            printf("%s \n", __PRETTY_FUNCTION__);

            if (args->action == AKEY_EVENT_ACTION_DOWN)
            {
                if (args->keyCode == AKEYCODE_VOLUME_UP)
                {   
                    manager->lock();
                    manager->switch_focus_to_next_application_locked();
                    manager->unlock();
                }
            }
        }

        void notifyMotion(const android::NotifyMotionArgs* args)
        {
            (void) args;
        }

        void notifySwitch(const android::NotifySwitchArgs* args)
        {
            (void) args;
        }

        void notifyDeviceReset(const android::NotifyDeviceResetArgs* args)
        {
            (void) args;
        }

        ApplicationManager* manager;
    };

    class LockingIterator : public android::RefBase
    {
      public:
        void advance() 
        {
            it += 1;
        }

        bool is_valid() const
        {
            return it < manager->apps.size();
        }

        void make_current()
        {
            printf("%s \n", __PRETTY_FUNCTION__);
        }
        
        const android::sp<ApplicationSession>& operator*()
        {
            return manager->apps.valueAt(it);
        }

      protected:
        friend class ApplicationManager;
        LockingIterator(
            ApplicationManager* manager,
            size_t index) : manager(manager),
                            it(index)
        {
        }

        virtual ~LockingIterator() 
        {
            manager->unlock();
        }
        
        ApplicationManager* manager;
        size_t it;
    };

    ApplicationManager() : input_listener(new InputListener(this)),
                           input_setup(new android::InputSetup(input_listener)),
                           focused_application(0)
    {
        input_setup->input_reader_thread->run();
        input_setup->looper_thread->run();
    }

    // From DeathRecipient
    void binderDied(const android::wp<android::IBinder>& who)
    {
        printf("%s \n", __PRETTY_FUNCTION__);
        android::Mutex::Autolock al(guard);
        android::sp<android::IBinder> sp = who.promote();
        size_t idx = apps.indexOfKey(sp);
        if (idx == focused_application)
        {
            switch_focus_to_next_application_locked();
        }
        
        apps.removeItem(sp);
    }

    void lock()
    {
        guard.lock();
    }

    void unlock()
    {
        guard.unlock();
    }

    android::sp<LockingIterator> iterator()
    {
        lock();
        android::sp<LockingIterator> it(new LockingIterator(this, 0));
        
        return it;
    }

    void start_a_new_session(const android::String8& app_name,
                             const android::sp<android::IApplicationManagerSession>& session,
                             int ashmem_fd,
                             int out_socket_fd,
                             int in_socket_fd)
    {
        printf("%s \n", __PRETTY_FUNCTION__);
        printf("\t%s \n", app_name.string());
        printf("\t%d \n", ashmem_fd);
        printf("\t%d \n", out_socket_fd);
        printf("\t%d \n", in_socket_fd);

        
        android::sp<ApplicationSession> app_session(new ApplicationSession(
            session,
            app_name,
            android::sp<android::InputChannel>(
                new android::InputChannel(
                    app_name,
                    ashmem_fd,
                    out_socket_fd,
                    in_socket_fd))));
        
        {
            android::Mutex::Autolock al(guard);
            session->asBinder()->linkToDeath(
                android::sp<android::IBinder::DeathRecipient>(this));                
            apps.add(session->asBinder(), app_session);
            switch_focused_application_locked(apps.size() - 1);
        }

        printf("Iterating registered applications now:\n");
        android::sp<LockingIterator> it = iterator();
        while(it->is_valid())
        {
            printf("\t %s \n", (**it)->app_name.string());
            it->advance();
        }
    }
    
    void switch_focused_application_locked(size_t index_of_next_focused_app)
    {
        if (focused_application < apps.size())
        {
            apps.valueAt(focused_application)->remote_session->raise_application_surfaces_to_layer(non_focused_application_layer);
        }

        focused_application = index_of_next_focused_app;

        if (focused_application < apps.size())
        {
            apps.valueAt(focused_application)->remote_session->raise_application_surfaces_to_layer(focused_application_base_layer);
        }
    }

    void switch_focus_to_next_application_locked()
    {
        size_t new_idx = focused_application + 1;
        if (new_idx >= apps.size())
            new_idx = 0;

        switch_focused_application_locked(new_idx);
    }

    android::sp<android::InputListenerInterface> input_listener;
    android::sp<android::InputSetup> input_setup;
    android::Mutex guard;
    android::KeyedVector< android::sp<android::IBinder>, android::sp<ApplicationSession> > apps;
    size_t focused_application;
};

}

int main(int argc, char** argv)
{
    android::sp<ApplicationManager> app_manager(new ApplicationManager());
    android::sp<android::IServiceManager> service_manager = android::defaultServiceManager();
    if (android::NO_ERROR != service_manager->addService(
            android::String16(android::IApplicationManager::exported_service_name()),
            app_manager))
    {
        printf("Error registering service with the system ... exiting now.");
        return EXIT_FAILURE;
    }

    android::ProcessState::self()->startThreadPool();
    android::IPCThreadState::self()->joinThreadPool();
}
