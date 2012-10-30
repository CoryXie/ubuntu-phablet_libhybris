#ifndef INPUT_CONSUMER_THREAD_H_
#define INPUT_CONSUMER_THREAD_H_

#include <ui/InputTransport.h>
#include <utils/Looper.h>

namespace android
{

struct InputConsumerThread : public android::Thread
{
InputConsumerThread(android::InputConsumer& input_consumer) 
        : input_consumer(input_consumer),
          looper(android::Looper::prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS))
{
looper->addFd(input_consumer.getChannel()->getReceivePipeFd(),
                  input_consumer.getChannel()->getReceivePipeFd(),
                  ALOOPER_EVENT_INPUT,
                  NULL,
                  NULL);
                         
}

        bool threadLoop()
        {
while (true)
{
switch(looper->pollOnce(5 * 1000))
{
    case ALOOPER_POLL_TIMEOUT:
    case ALOOPER_POLL_ERROR:
continue;
break;
}

// printf("%s \n", __PRETTY_FUNCTION__);
InputEvent* event = NULL;
bool result = true;
switch(input_consumer.consume(&event_factory, &event))
{
    case OK:
//TODO:Dispatch to input listener
result = true;
//printf("Yeah, we have an event client-side.\n");
input_consumer.sendFinishedSignal(result);
break;
    case INVALID_OPERATION:
result = true;
break;
    case NO_MEMORY:
result = true;
break;
}                               
}
return true;
}
        
                android::InputConsumer input_consumer;
android::sp<android::Looper> looper;
android::PreallocatedInputEventFactory event_factory;
};
}
#endif // INPUT_CONSUMER_THREAD_H_
