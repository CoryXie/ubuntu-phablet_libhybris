#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include <utils/Looper.h>
#include <utils/threads.h>

namespace ubuntu
{
namespace application
{
struct EventLoop : public android::Thread
{
    EventLoop(const android::sp<android::Looper>& looper) : looper(looper)
    {
    }

    bool threadLoop()
    {            
        bool result = true;
        while(true)
        {
            switch(looper->pollOnce(5*1000))
            {
                case ALOOPER_POLL_CALLBACK:
                case ALOOPER_POLL_TIMEOUT:
                    result = true;
                    break;
                case ALOOPER_POLL_ERROR:
                    result = false;
                    break;
            }
        }

        return result;
    }

    android::sp<android::Looper> looper;
};
}
}

#endif // EVENT_LOOP_H_
