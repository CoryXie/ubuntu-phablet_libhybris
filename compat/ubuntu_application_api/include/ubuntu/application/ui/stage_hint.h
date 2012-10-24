#ifndef UBUNTU_APPLICATION_UI_STAGE_HINT_H_
#define UBUNTU_APPLICATION_UI_STAGE_HINT_H_

#include "ubuntu/application/ui/ubuntu_application_ui.h"

namespace ubuntu
{
namespace application
{
namespace ui
{
enum StageHint
{
    main_stage = MAIN_STAGE_HINT,
    integration_stage = INTEGRATION_STAGE_HINT,
    share_stage = SHARE_STAGE_HINT,
    content_picking_stage = CONTENT_PICKING_STAGE_HINT,
    side_stage = SIDE_STAGE_HINT,
    configuration_stage = CONFIGURATION_STAGE_HINT
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_STAGE_HINT_H_
