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
#ifndef UBUNTU_UI_SESSION_SERVICE_H_
#define UBUNTU_UI_SESSION_SERVICE_H_

#include "ubuntu/application/ui/session.h"
#include "ubuntu/platform/shared_ptr.h"
#include "ubuntu/ui/session_enumerator.h"

namespace ubuntu
{
namespace application
{
namespace ui
{
class Session;
class SessionCredentials;
}
}
namespace ui
{
class SessionService : public platform::ReferenceCountedBase
{
public:
    typedef platform::shared_ptr<SessionService> Ptr;

    static const Ptr& instance();

    virtual ~SessionService() {}

    virtual const ubuntu::application::ui::Session::Ptr& start_a_new_session(const ubuntu::application::ui::SessionCredentials& cred) = 0;

    virtual void install_session_lifecycle_observer(const SessionLifeCycleObserver::Ptr& observer) = 0;

    virtual void focus_running_session_with_id(int id) = 0;

protected:
    SessionService() {}
    SessionService(const SessionService&) = delete;
    SessionService& operator=(const SessionService&) = delete;
};
}
}

#endif // UBUNTU_UI_SESSION_SERVICE_H_
