#ifndef UBUNTU_APPLICATION_SESSION_H_
#define UBUNTU_APPLICATION_SESSION_H_

#include "ubuntu/platform/shared_ptr.h"

namespace ubuntu
{
namespace application
{
class Session : public ubuntu::platform::ReferenceCountedBase
{
  public:
    typedef ubuntu::platform::shared_ptr<Session> Ptr;

  protected:
    Session() {}
    virtual ~Session() {}

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
};
}
}

#endif // UBUNTU_APPLICATION_SESSION_H_
