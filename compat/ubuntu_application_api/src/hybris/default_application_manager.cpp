#include "default_application_manager.h"

#include "default_application_manager_input_setup.h"
#include "default_application_session.h"
#include "default_shell.h"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>

#include <input/InputListener.h>
#include <input/InputReader.h>
#include <ui/InputTransport.h>
#include <utils/threads.h>

#include <cstdio>

namespace mir
{

ApplicationManager::InputFilter::InputFilter(ApplicationManager* manager) : manager(manager)
{
}

bool ApplicationManager::InputFilter::filter_event(const android::InputEvent* event)
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

bool ApplicationManager::InputFilter::handle_key_event(const android::KeyEvent* event)
{
    //printf("%s: %p\n", __PRETTY_FUNCTION__, event);
    
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

ApplicationManager::LockingIterator::LockingIterator(
    ApplicationManager* manager,
    size_t index) : manager(manager),
                    it(index)
{
}

void ApplicationManager::LockingIterator::advance() 
{
    it += 1;
}

bool ApplicationManager::LockingIterator::is_valid() const
{
    return it < manager->apps.size();
}

void ApplicationManager::LockingIterator::make_current()
{
    //printf("%s \n", __PRETTY_FUNCTION__);
}
        
const android::sp<mir::ApplicationSession>& ApplicationManager::LockingIterator::operator*()
{
    return manager->apps.valueFor(manager->apps_as_added[it]);
}

ApplicationManager::LockingIterator::~LockingIterator() 
{
    manager->unlock();
}
        
ApplicationManager::ApplicationManager() : input_filter(new InputFilter(this)),
                                           input_setup(new android::InputSetup(input_filter)),
                                           focused_application(0)
{
    input_setup->start();
}

// From DeathRecipient
void ApplicationManager::binderDied(const android::wp<android::IBinder>& who)
{
    //printf("%s \n", __PRETTY_FUNCTION__);
    android::Mutex::Autolock al(guard);
    android::sp<android::IBinder> sp = who.promote();

    size_t i = 0;
    for(i = 0; i < apps_as_added.size(); i++)
    {
        if (apps_as_added[i] == sp)
            break;
    }

    size_t next_focused_app = 0;
    next_focused_app = apps_as_added.removeAt(i);        
    apps.removeItem(sp);

    if (next_focused_app >= apps_as_added.size())
        next_focused_app = 0;

    if (i == focused_application)
    {              
        switch_focused_application_locked(next_focused_app);
    } else if(focused_application > i)
    {
        focused_application--;
    }    
    
}

void ApplicationManager::lock()
{
    guard.lock();
}

void ApplicationManager::unlock()
{
    guard.unlock();
}

android::sp<ApplicationManager::LockingIterator> ApplicationManager::iterator()
{
    lock();
    android::sp<ApplicationManager::LockingIterator> it(
        new ApplicationManager::LockingIterator(this, 0));
        
    return it;
}

void ApplicationManager::start_a_new_session(const android::String8& app_name,
                         const android::sp<android::IApplicationManagerSession>& session,
                         int ashmem_fd,
                         int out_socket_fd,
                         int in_socket_fd)
{
    //printf("%s \n", __PRETTY_FUNCTION__);
    //printf("\t%s \n", app_name.string());
    //printf("\t%d \n", ashmem_fd);
    //printf("\t%d \n", out_socket_fd);
    //printf("\t%d \n", in_socket_fd);
    
    android::sp<mir::ApplicationSession> app_session(new mir::ApplicationSession(
        session,
        app_name));
    {
        android::Mutex::Autolock al(guard);
        session->asBinder()->linkToDeath(
            android::sp<android::IBinder::DeathRecipient>(this));                
        apps.add(session->asBinder(), app_session);            
        apps_as_added.push_back(session->asBinder());
        // switch_focused_application_locked(apps.indexOfKey(session->asBinder()));
        // switch_focused_application_locked(apps_as_added.size() - 1);
    }
    
    //printf("Iterating registered applications now:\n");
    android::sp<LockingIterator> it = iterator();
    while(it->is_valid())
    {
        //printf("\t %s \n", (**it)->app_name.string());
        it->advance();
    }
}

void ApplicationManager::register_a_surface(const android::String8& title,
                                            const android::sp<android::IApplicationManagerSession>& session,
                                            int32_t token,
                                            int ashmem_fd,
                                            int out_socket_fd,
                                            int in_socket_fd)
{
    //printf("%s \n", __PRETTY_FUNCTION__);
    //printf("\t%s \n", title.string());
    //printf("\t%d \n", ashmem_fd);
    //printf("\t%d \n", out_socket_fd);
    //printf("\t%d \n", in_socket_fd);
    
    android::Mutex::Autolock al(guard);
    android::sp<android::InputChannel> input_channel(
        new android::InputChannel(
            title,
            dup(ashmem_fd),
            dup(in_socket_fd),
            dup(out_socket_fd)));
    
    android::sp<mir::ApplicationSession::Surface> surface(
        new mir:: ApplicationSession::Surface(
            apps.valueFor(session->asBinder()).get(), 
            input_channel, 
            token));
    
    input_setup->input_manager->getDispatcher()->registerInputChannel(
        surface->input_channel,
        surface->make_input_window_handle(),
        false);
    apps.valueFor(session->asBinder())->register_surface(surface);
    
    size_t i = 0;
    for(i = 0; i < apps_as_added.size(); i++)
    {
        if (apps_as_added[i] == session->asBinder())
            break;
    }
    
    switch_focused_application_locked(i);
    
}

void ApplicationManager::switch_focused_application_locked(size_t index_of_next_focused_app)
{
    //printf("%s: %d vs. current: %d \n", 
    //       __PRETTY_FUNCTION__, 
    //       index_of_next_focused_app, 
    //       focused_application);
    
    if (apps.size() > 1 && 
        focused_application < apps.size() &&
        focused_application != index_of_next_focused_app)
    {
        //printf("\tLowering current application now for idx: %d \n", focused_application);
        apps.valueFor(apps_as_added[focused_application])->raise_application_surfaces_to_layer(non_focused_application_layer);
    }
    
    focused_application = index_of_next_focused_app;
    
    if (focused_application < apps.size())
    {
        //printf("\tRaising application now for idx: %d \n", focused_application);
        
        apps.valueFor(apps_as_added[focused_application])->raise_application_surfaces_to_layer(focused_application_base_layer);
        input_setup->input_manager->getDispatcher()->setFocusedApplication(
            apps.valueFor(apps_as_added[focused_application])->input_application_handle());
        input_setup->input_manager->getDispatcher()->setInputWindows(
            apps.valueFor(apps_as_added[focused_application])->input_window_handles());
    }
}

void ApplicationManager::switch_focus_to_next_application_locked()
{
    size_t new_idx = focused_application + 1;
    if (new_idx >= apps.size())
        new_idx = 0;
    
    //printf("current: %d, next: %d \n", focused_application, new_idx);
    
    switch_focused_application_locked(new_idx);
}

}

int main(int argc, char** argv)
{
    android::sp<mir::ApplicationManager> app_manager(new mir::ApplicationManager());
    
    // Register service
    android::sp<android::IServiceManager> service_manager = android::defaultServiceManager();
    if (android::NO_ERROR != service_manager->addService(
            android::String16(android::IApplicationManager::exported_service_name()),
            app_manager))
    {
        //printf("Error registering service with the system ... exiting now.");
        return EXIT_FAILURE;
    }

    android::ProcessState::self()->startThreadPool();
    android::IPCThreadState::self()->joinThreadPool();
}
