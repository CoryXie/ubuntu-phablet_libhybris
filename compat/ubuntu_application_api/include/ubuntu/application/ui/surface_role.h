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
