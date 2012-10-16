#ifndef UBUNTU_APPLICATION_UI_SURFACE_H_
#define UBUNTU_APPLICATION_UI_SURFACE_H_

#include "ubuntu/platform/shared_ptr.h"

#include "ubuntu/application/ui/input/listener.h"

#include <EGL/egl.h>

namespace ubuntu
{
namespace application
{
namespace ui
{
class Surface : public ubuntu::platform::ReferenceCountedBase
{
  public:
    typedef ubuntu::platform::shared_ptr<Surface> Ptr;
    
    // Default surface API
    virtual bool is_visible() const = 0;
    virtual void set_visible(bool visible) = 0; 

    virtual void set_alpha(float alpha) = 0;
    virtual float alpha() const = 0;

    virtual void move_to(int x, int y) = 0;
    virtual void move_by(int dx, int dy) = 0;

    // Bind to EGL/GL rendering API
    virtual EGLNativeWindowType to_native_window_type() = 0;

  protected:
    Surface(const input::Listener::Ptr& input_listener) {}
    virtual ~Surface() {}

    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;

    const input::Listener::Ptr& registered_input_listener() const
    {
        return input_listener;
    }

  private:
    input::Listener::Ptr input_listener;
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_SURFACE_H_
