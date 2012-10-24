#ifndef UBUNTU_APPLICATION_UI_SESSION_H_
#define UBUNTU_APPLICATION_UI_SESSION_H_

#include "ubuntu/application/session.h"
#include "ubuntu/application/ui/physical_display_info.h"
#include "ubuntu/application/ui/surface.h"
#include "ubuntu/application/ui/surface_properties.h"
#include "ubuntu/platform/shared_ptr.h"

#include <EGL/egl.h>

namespace ubuntu
{
namespace application
{
namespace ui
{

class Session : public ubuntu::application::Session
{
  public:
    typedef ubuntu::platform::shared_ptr<Session> Ptr;

    virtual PhysicalDisplayInfo::Ptr physical_display_info(PhysicalDisplayIdentifier id) = 0;

    virtual Surface::Ptr create_surface(
        const SurfaceProperties& props, 
        const ubuntu::application::ui::input::Listener::Ptr& listener) = 0;
    virtual void destroy_surface(const Surface::Ptr& surface) = 0;

  protected:
    Session() {}
    virtual ~Session() {}
    
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_SESSION_H_
