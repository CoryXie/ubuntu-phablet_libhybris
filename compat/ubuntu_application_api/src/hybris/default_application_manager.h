#ifndef DEFAULT_APPLICATION_MANAGER_H_
#define DEFAULT_APPLICATION_MANAGER_H_

#include "application_manager.h"
#include "default_application_manager_input_setup.h"
#include "default_application_session.h"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>

#include <input/InputListener.h>
#include <input/InputReader.h>
#include <ui/InputTransport.h>
#include <utils/threads.h>

namespace mir
{
struct ApplicationManager : public android::BnApplicationManager,
                            public android::IBinder::DeathRecipient
{
    static const int shell_components_base_layer = 200;
    static const int focused_application_base_layer = 100;
    static const int wallpaper_layer = 0;
    static const int non_focused_application_layer = -1;

    class InputFilter : public android::InputFilter
    {
      public:
        InputFilter(ApplicationManager* manager);
        
        bool filter_event(const android::InputEvent* event);
                
      private:
        ApplicationManager* manager;

        bool handle_key_event(const android::KeyEvent* event);
    };

    class LockingIterator : public android::RefBase
    {
      public:
        void advance() ;

        bool is_valid() const;

        void make_current();
        
        const android::sp<mir::ApplicationSession>& operator*();

      protected:
        friend class ApplicationManager;

        LockingIterator(
            ApplicationManager* manager,
            size_t index);

        virtual ~LockingIterator();
        
      private:
        ApplicationManager* manager;
        size_t it;
    };

    ApplicationManager();

    // From DeathRecipient
    void binderDied(const android::wp<android::IBinder>& who);

    void lock();

    void unlock();

    android::sp<LockingIterator> iterator();

    void start_a_new_session(const android::String8& app_name,
                             const android::sp<android::IApplicationManagerSession>& session,
                             int ashmem_fd,
                             int out_socket_fd,
                             int in_socket_fd);
    
    void register_a_surface(const android::String8& title,
                            const android::sp<android::IApplicationManagerSession>& session,
                            int32_t token,
                            int ashmem_fd,
                            int out_socket_fd,
                            int in_socket_fd);
    
    void switch_focused_application_locked(size_t index_of_next_focused_app);
    void switch_focus_to_next_application_locked();

  private:
    android::sp<android::InputListenerInterface> input_listener;
    android::sp<InputFilter> input_filter;
    android::sp<android::InputSetup> input_setup;
    android::Mutex guard;
    android::KeyedVector< android::sp<android::IBinder>, android::sp<mir::ApplicationSession> > apps;
    android::Vector< android::sp<android::IBinder> > apps_as_added;
    size_t focused_application;
};

}

#endif // DEFAULT_APPLICATION_MANAGER_H_
