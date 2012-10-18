#ifndef UBUNTU_APPLICATION_UI_SURFACE_FACTORY_H_
#define UBUNTU_APPLICATION_UI_SURFACE_FACTORY_H_

#include "ubuntu/platform/shared_ptr.h"

namespace ubuntu
{
namespace application
{
namespace ui
{
class SurfaceProperties;

class SurfaceFactory : public ubuntu::platform::ReferenceCountedBase
{
  public:
    typedef ubuntu::platform::shared_ptr<SurfaceFactory> Ptr;
 
    static const Ptr& instance();

    virtual Surface::Ptr create_surface(
        const ubuntu::application::ui::SurfaceProperties& props, 
        const ubuntu::application::ui::input::Listener::Ptr& listener) = 0;
    
  protected:
    SurfaceFactory() {}
    virtual ~SurfaceFactory() {}
    
    SurfaceFactory(const SurfaceFactory&) = delete;
    SurfaceFactory& operator=(const SurfaceFactory&) = delete;
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_SURFACE_FACTORY_H_
