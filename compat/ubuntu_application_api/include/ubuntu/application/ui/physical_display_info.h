#ifndef UBUNTU_APPLICATION_UI_PHYSICAL_DISPLAY_INFO_H_
#define UBUNTU_APPLICATION_UI_PHYSICAL_DISPLAY_INFO_H_

#include "ubuntu/platform/shared_ptr.h"

namespace ubuntu
{
namespace application
{
namespace ui
{

enum PhysicalDisplayIdentifier
{
    first_physical_display = 0,
    second_physical_display = 1,
    third_physical_display = 2,
    fourth_physical_display = 3,
    fifth_physical_display = 4,
    sixth_physical_display = 5,
    seventh_physical_display = 6,
    eigth_physical_display = 7,
    ninth_physical_display = 8,
    tenth_physical_display = 9,

    primary_physical_display = first_physical_display
};

class PhysicalDisplayInfo : public ubuntu::platform::ReferenceCountedBase
{
  public:
    typedef ubuntu::platform::shared_ptr<PhysicalDisplayInfo> Ptr;

    virtual int dpi() = 0;
    virtual int horizontal_resolution() = 0;
    virtual int vertical_resolution() = 0;

  protected:
    PhysicalDisplayInfo() {}
    virtual ~PhysicalDisplayInfo() {}

    PhysicalDisplayInfo(const PhysicalDisplayInfo&) = delete;
    PhysicalDisplayInfo& operator=(const PhysicalDisplayInfo&) = delete;
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_PHYSICAL_DISPLAY_INFO_H_
