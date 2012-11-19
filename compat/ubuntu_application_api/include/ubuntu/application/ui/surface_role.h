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
#ifndef UBUNTU_APPLICATION_UI_SURFACE_ROLE_H_
#define UBUNTU_APPLICATION_UI_SURFACE_ROLE_H_

#include "ubuntu/application/ui/ubuntu_application_ui.h"

namespace ubuntu
{
namespace application
{
namespace ui
{
enum SurfaceRole
{
    main_actor_role = MAIN_ACTOR_ROLE,
    tool_support_actor_role = TOOL_SUPPORT_ACTOR_ROLE,
    dialog_support_actor_role = DIALOG_SUPPORT_ACTOR_ROLE
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_SURFACE_ROLE_H_
