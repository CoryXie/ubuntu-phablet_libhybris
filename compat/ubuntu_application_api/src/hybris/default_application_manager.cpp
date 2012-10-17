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
                  input_window_handle(new InputWindowHandle(this))
        {
            input_window_handles.push_back(input_window_handle);
        }

        struct InputApplicationHandle : public android::InputApplicationHandle
        {
            InputApplicationHandle(ApplicationSession* parent) : parent(parent)
            {
                mInfo = new android::InputApplicationInfo();
                mInfo->name = parent->app_name;
                mInfo->dispatchingTimeout = 10 * 1000 * 1000; // TODO(tvoss): Find out sensible value here
            }

            bool updateInfo()
            {
                return true;
            }

            ApplicationSession* parent;
        };

        struct InputWindowHandle : public android::InputWindowHandle
        {
            InputWindowHandle(ApplicationSession* parent) 
                    : android::InputWindowHandle(
                        android::sp<InputApplicationHandle>(
                            new InputApplicationHandle(parent))),
                      parent(parent)
            {
                SkRegion touchable_region;
                touchable_region.setRect(0, 0, 720, 960);

                mInfo = new android::InputWindowInfo();
                mInfo->name = parent->app_name;
                mInfo->touchableRegion = touchable_region;
                mInfo->frameLeft = 0;
                mInfo->frameTop = 0;
                mInfo->frameRight = 720;
                mInfo->frameBottom = 960;
                mInfo->scaleFactor = 1.f;
                mInfo->visible = true;
                mInfo->canReceiveKeys = true;
                mInfo->hasFocus = true;
                mInfo->hasWallpaper = false;
                mInfo->paused = false;
                mInfo->layer = 100;
                mInfo->ownerPid = 0;
                mInfo->ownerUid = 0;
                mInfo->inputFeatures = 0;
                mInfo->inputChannel = parent->input_channel;
            }

            bool updateInfo()
            {
                return true;
            }

            ApplicationSession* parent;
        };

        android::sp<android::IApplicationManagerSession> remote_session;
        android::String8 app_name;
        android::sp<android::InputChannel> input_channel;
        android::sp<android::InputWindowHandle> input_window_handle;
        android::Vector< android::sp<android::InputWindowHandle> > input_window_handles;
    };

    class InputFilter : public android::InputFilter
    {
      public:
        InputFilter(ApplicationManager* manager) : manager(manager)
        {
        }

        bool filter_event(const android::InputEvent* event)
        {
            bool result = true;
            
            switch (event->getType())
            {
                case AINPUT_EVENT_TYPE_KEY:
                    result = handle_key_event(static_cast<const android::KeyEvent*>(event));
                    break;
            }

            return result;
        }

        bool handle_key_event(const android::KeyEvent* event)
        {
            printf("%s: %p\n", __PRETTY_FUNCTION__, event);

            bool result = true;

            if (!event)
                return result;

            if (event->getAction() == AKEY_EVENT_ACTION_DOWN)
            {
                if (event->getKeyCode() == AKEYCODE_VOLUME_UP)
                {   
                    manager->lock();
                    manager->switch_focus_to_next_application_locked();
                    manager->unlock();
                    result = false;
                }
            }

            return result;
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

    ApplicationManager() : input_filter(new InputFilter(this)),
                           input_setup(new android::InputSetup(input_filter)),
                           focused_application(0)
    {
        input_setup->start();
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
                    dup(ashmem_fd),
                    dup(in_socket_fd),
                    dup(out_socket_fd)))));
        
        {
            android::Mutex::Autolock al(guard);
            session->asBinder()->linkToDeath(
                android::sp<android::IBinder::DeathRecipient>(this));                
            apps.add(session->asBinder(), app_session);            

            input_setup->input_manager->getDispatcher()->registerInputChannel(
                app_session->input_channel,
                app_session->input_window_handle,
                false);

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
        if (apps.size() > 1 && focused_application < apps.size())
        {
            apps.valueAt(focused_application)->remote_session->raise_application_surfaces_to_layer(non_focused_application_layer);
        }

        focused_application = index_of_next_focused_app;

        if (focused_application < apps.size())
        {
            apps.valueAt(focused_application)->remote_session->raise_application_surfaces_to_layer(focused_application_base_layer);
            input_setup->input_manager->getDispatcher()->setFocusedApplication(
                apps.valueAt(focused_application)->input_window_handle->inputApplicationHandle);
            input_setup->input_manager->getDispatcher()->setInputWindows(
                apps.valueAt(focused_application)->input_window_handles);

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
    android::sp<InputFilter> input_filter;
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
