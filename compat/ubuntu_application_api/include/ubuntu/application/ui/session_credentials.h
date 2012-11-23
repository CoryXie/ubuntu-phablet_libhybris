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
#ifndef UBUNTU_APPLICATION_UI_SESSION_CREDENTIALS_H_
#define UBUNTU_APPLICATION_UI_SESSION_CREDENTIALS_H_

#include "ubuntu_application_ui.h"

#include <cstdio>

namespace ubuntu
{
namespace application
{
namespace ui
{
enum SessionType
{
    user_session_type = USER_SESSION_TYPE,
    system_session_type = SYSTEM_SESSION_TYPE
};

struct SessionCredentials
{
    SessionCredentials(SessionType session_type, const char* app_name) : session_type(session_type)
    {
        strncpy(application_name, app_name, max_application_name_length); 
    }

    SessionCredentials() : session_type(user_session_type)
    {
        snprintf(
            application_name, 
            max_application_name_length,
            "unknown_application");
    }
    
    SessionType session_type;
    enum { max_application_name_length = 512 };
    char application_name[max_application_name_length];
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_SESSION_CREDENTIALS_H_
