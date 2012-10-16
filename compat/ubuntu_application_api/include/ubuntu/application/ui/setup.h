#ifndef UBUNTU_APPLICATION_UI_SETUP_H_
#define UBUNTU_APPLICATION_UI_SETUP_H_

#include "ubuntu/application/ui/stage_hint.h"
#include "ubuntu/application/ui/form_factor_hint.h"
#include "ubuntu/platform/shared_ptr.h"

namespace ubuntu
{
namespace application
{
namespace ui
{
class Setup : public ubuntu::platform::ReferenceCountedBase
{
  public:
    typedef ubuntu::platform::shared_ptr<Setup> Ptr;

    static const Ptr& instance();

    virtual StageHint stage_hint() = 0;
    
    virtual FormFactorHintFlags form_factor_hint() = 0;

  protected:
    Setup() {}
    virtual ~Setup() {}

    Setup(const Setup&) = delete;
    Setup& operator=(const Setup&) = delete;
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_SETUP_H_
