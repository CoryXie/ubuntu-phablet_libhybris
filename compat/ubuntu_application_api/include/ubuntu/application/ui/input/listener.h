#ifndef UBUNTU_APPLICATION_UI_INPUT_LISTENER_H_
#define UBUNTU_APPLICATION_UI_INPUT_LISTENER_H_

#include "ubuntu/application/ui/input/event.h"
#include "ubuntu/platform/shared_ptr.h"

struct Event;

namespace ubuntu
{
namespace application
{
namespace ui
{
namespace input
{
class Listener : public ubuntu::platform::ReferenceCountedBase
{
  public:
    typedef ubuntu::platform::shared_ptr<Listener> Ptr;

    virtual void on_new_event(const Event& event) = 0;

  protected:
    Listener() {}
    virtual ~Listener() {}

    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;
};
}
}
}
}

#endif // UBUNTU_APPLICATION_UI_INPUT_LISTENER_H_
