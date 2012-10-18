#ifndef UBUNTU_APPLICATION_UI_FORM_FACTOR_HINT_H_
#define UBUNTU_APPLICATION_UI_FORM_FACTOR_HINT_H_

#include "ubuntu/application/ui/ubuntu_application_ui.h"

namespace ubuntu
{
namespace application
{
namespace ui
{
enum FormFactorHint
{
    desktop_form_factor = DESKTOP_FORM_FACTOR_HINT,
    phone_form_factor = PHONE_FORM_FACTOR_HINT,
    tablet_form_factor = TABLET_FORM_FACTOR_HINT
};

typedef unsigned int FormFactorHintFlags;

}
}
}

#endif // UBUNTU_APPLICATION_UI_FORM_FACTOR_HINT_H_
