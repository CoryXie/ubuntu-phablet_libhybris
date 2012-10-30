#ifndef UBUNTU_PLATFORM_EMPTY_BASE_H_
#define UBUNTU_PLATFORM_EMPTY_BASE_H_

namespace ubuntu
{
namespace platform
{
class EmptyBase
{
public:
    virtual ~EmptyBase() {}
protected:
    EmptyBase() {}
    EmptyBase(const EmptyBase&) = delete;
    EmptyBase& operator=(const EmptyBase&) = delete;
};
}
}

#endif // UBUNTU_PLATFORM_EMPTY_BASE_H_
