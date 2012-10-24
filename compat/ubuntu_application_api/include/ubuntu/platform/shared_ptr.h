#ifndef UBUNTU_PLATFORM_SHARED_PTR_H_
#define UBUNTU_PLATFORM_SHARED_PTR_H_

#ifdef ANDROID
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#else
#include <memory>
#endif

#include "empty_base.h"

namespace ubuntu
{
namespace platform
{

#ifdef ANDROID
typedef android::RefBase ReferenceCountedBase;

template<typename T>
struct shared_ptr : public android::sp<T>
{
    shared_ptr() : android::sp<T>()
    {
    }

    template<typename Y>
    shared_ptr(Y* p) : android::sp<T>(p)
    {
    }
};
#else 
typedef ubuntu::platform::EmptyBase ReferenceCountedBase;
using std::shared_ptr;
#endif

}
}

#endif // UBUNTU_PLATFORM_SHARED_PTR_H_
