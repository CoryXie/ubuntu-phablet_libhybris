/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
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
struct ApplicationManager : 
        public android::BnApplicationManager,
        public android::IBinder::DeathRecipient
{
    static const int default_shell_component_layer = 1000000;
    
    static const int default_dash_layer = default_shell_component_layer + 1;
    static const int default_launcher_layer = default_shell_component_layer + 2;
    static const int default_top_bar_layer = default_shell_component_layer + 3;
    static const int default_switcher_layer = default_shell_component_layer + 4;
    static const int default_osk_layer = default_shell_component_layer + 5;
    
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

    void start_a_new_session(
        int32_t session_type,
        const android::String8& app_name,
        const android::String8& desktop_file,
        const android::sp<android::IApplicationManagerSession>& session,
        int ashmem_fd,
        int out_socket_fd,
        int in_socket_fd);

    void register_a_surface(
        const android::String8& title,
        const android::sp<android::IApplicationManagerSession>& session,
        int32_t surface_role,
        int32_t token,
        int ashmem_fd,
        int out_socket_fd,
        int in_socket_fd);

    void register_an_observer(const android::sp<android::IApplicationManagerObserver>& observer);

    void switch_focused_application_locked(size_t index_of_next_focused_app);
    void switch_focus_to_next_application_locked();

private:
    void notify_observers_about_session_born(int id, const android::String8& desktop_file);
    void notify_observers_about_session_focused(int id, const android::String8& desktop_file);
    void notify_observers_about_session_died(int id, const android::String8& desktop_file);

    android::sp<android::InputListenerInterface> input_listener;
    android::sp<InputFilter> input_filter;
    android::sp<android::InputSetup> input_setup;
    android::Mutex guard;
    android::KeyedVector< android::sp<android::IBinder>, android::sp<mir::ApplicationSession> > apps;
    android::Vector< android::sp<android::IBinder> > apps_as_added;
    android::Mutex observer_guard;
    android::Vector< android::sp<android::IApplicationManagerObserver> > app_manager_observers;
    size_t focused_application;
};

}

#endif // DEFAULT_APPLICATION_MANAGER_H_
