#ifndef DEFAULT_APPLICATION_MANAGER_INPUT_SETUP_H_
#define DEFAULT_APPLICATION_MANAGER_INPUT_SETUP_H_

#include <input/InputListener.h>
#include <input/InputReader.h>
#include <input/PointerController.h>
#include <input/SpriteController.h>
#include <surfaceflinger/SurfaceComposerClient.h>

namespace android
{
class DefaultPointerControllerPolicy : public android::PointerControllerPolicyInterface
{
  public:

    static const size_t bitmap_width = 64;
    static const size_t bitmap_height = 64;

    DefaultPointerControllerPolicy()
    {
        bitmap.setConfig(
            SkBitmap::kARGB_8888_Config,
            bitmap_width,
            bitmap_height);
        bitmap.allocPixels();
        
        // Icon for spot touches
        bitmap.eraseARGB(125, 0, 255, 0);
        spotTouchIcon = android::SpriteIcon(
            bitmap, 
            bitmap_width/2, 
            bitmap_height/2);
        // Icon for anchor touches
        bitmap.eraseARGB(125, 0, 0, 255);
        spotAnchorIcon = android::SpriteIcon(
            bitmap, 
            bitmap_width/2, 
            bitmap_height/2);
        // Icon for hovering touches
        bitmap.eraseARGB(125, 255, 0, 0);
        spotHoverIcon = android::SpriteIcon(
            bitmap, 
            bitmap_width/2, 
            bitmap_height/2);
    }

    void loadPointerResources(android::PointerResources* outResources)
    {
        outResources->spotHover = spotHoverIcon.copy();
        outResources->spotTouch = spotTouchIcon.copy();
        outResources->spotAnchor = spotAnchorIcon.copy();
    }

    android::SpriteIcon spotHoverIcon;
    android::SpriteIcon spotTouchIcon;
    android::SpriteIcon spotAnchorIcon;
    SkBitmap bitmap;
};

class DefaultInputReaderPolicyInterface : public android::InputReaderPolicyInterface
{
  public:
    static const android::DisplayID internal_display_id = 0;
    static const android::DisplayID external_display_id = 1;

    DefaultInputReaderPolicyInterface(const android::sp<android::Looper>& looper) 
            : looper(looper),
              default_layer_for_touch_point_visualization(INT_MAX)
    {
        default_configuration.showTouches = true;

        android::DisplayInfo info;
        android::SurfaceComposerClient::getDisplayInfo(
            internal_display_id,
            &info);

        default_configuration.setDisplayInfo(
            internal_display_id,
            false, /* external */
            info.w,
            info.h,
            info.orientation);

        /*android::SurfaceComposerClient::getDisplayInfo(
          external_display_id,
          &default_configuration.mExternalDisplay);
        
          default_configuration.mInternalDisplay.width = info.width;
          default_configuration.mInternalDisplay.height = info.height;
          default_configuratoin.mInternalDisplay.orientation = info.orientation;
        */
    }

    void getReaderConfiguration(android::InputReaderConfiguration* outConfig)
    {
        *outConfig = default_configuration;
    }
    
    android::sp<android::PointerControllerInterface> obtainPointerController(int32_t deviceId)
    {
        (void) deviceId;
        
        android::sp<android::SpriteController> sprite_controller(
            new android::SpriteController(
                looper, 
                default_layer_for_touch_point_visualization));
        android::sp<android::PointerController> pointer_controller(
            new android::PointerController(
                android::sp<DefaultPointerControllerPolicy>(new DefaultPointerControllerPolicy()),
                looper,
                sprite_controller));
        pointer_controller->setPresentation(
            android::PointerControllerInterface::PRESENTATION_SPOT);
        int32_t w, h, o;
        default_configuration.getDisplayInfo(internal_display_id,
                                             false,
                                             &w,
                                             &h,
                                             &o);
        pointer_controller->setDisplaySize(w, h);
        return pointer_controller;
    }
  private:
    android::sp<android::Looper> looper;
    int default_layer_for_touch_point_visualization;
    android::InputReaderConfiguration default_configuration;
};

class LooperThread : public android::Thread
{
  public:
    static const int default_poll_timeout_ms = 500;

    LooperThread(const android::sp<android::Looper>& looper) : looper(looper)
    {        
    }

  private:
    bool threadLoop()
    {
        if (ALOOPER_POLL_ERROR == looper->pollAll(default_poll_timeout_ms))
            return false;
        return true;
    }
    
    android::sp<android::Looper> looper;
};

struct InputSetup : public android::RefBase
{
    InputSetup(const sp<InputListenerInterface>& listener)          
            : looper(new android::Looper(false)),
              looper_thread(new LooperThread(looper)),
              event_hub(new android::EventHub()),
              input_reader_policy(new DefaultInputReaderPolicyInterface(looper)),
              input_listener(listener),
              input_reader(new android::InputReader(
                  event_hub, 
                  input_reader_policy,
                  input_listener)),
              input_reader_thread(new android::InputReaderThread(input_reader))
    {
    }

    ~InputSetup()
    {
        input_reader_thread->requestExitAndWait();
    }

    android::sp<android::Looper> looper;
    android::sp<LooperThread> looper_thread;
    
    android::sp<android::EventHubInterface> event_hub;
    android::sp<android::InputReaderPolicyInterface> input_reader_policy;
    android::sp<android::InputListenerInterface> input_listener;
    android::sp<android::InputReaderInterface> input_reader;
    android::sp<android::InputReaderThread> input_reader_thread;

    android::Condition wait_condition;
    android::Mutex wait_guard;
};

}

#endif // DEFAULT_APPLICATION_MANAGER_INPUT_SETUP_H_
