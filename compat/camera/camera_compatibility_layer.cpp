#include "camera_compatibility_layer.h"

#include <surface_flinger_compatibility_layer_internal.h>

#include <binder/ProcessState.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>

#define REPORT_FUNCTION() printf("%s \n", __PRETTY_FUNCTION__);

struct CameraControl : public android::CameraListener
{
    android::Mutex guard;
    CameraControlListener* listener;
    android::sp<android::Camera> camera;
    android::CameraParameters camera_parameters;

    void notify(int32_t msg_type, int32_t ext1, int32_t ext2)
    {
        REPORT_FUNCTION();
        printf("\text1: %d, ext2: %d \n", ext1, ext2);
        if (!listener)
            return;
        
        switch(msg_type)
        {
            case CAMERA_MSG_ERROR:
                if (listener->on_msg_error_cb)
                    listener->on_msg_error_cb(listener->context);
                break;
            case CAMERA_MSG_SHUTTER:
                if (listener->on_msg_shutter_cb)
                    listener->on_msg_shutter_cb(listener->context);
                break;
            case CAMERA_MSG_ZOOM:
                if (listener->on_msg_zoom_cb)
                    listener->on_msg_zoom_cb(listener->context, ext1);
                break;
            case CAMERA_MSG_FOCUS:
                if (listener->on_msg_focus_cb)
                    listener->on_msg_focus_cb(listener->context);
                break;
            default:
                break;
        }
    }
    
    void postData(
        int32_t msg_type, 
        const android::sp<android::IMemory>& data, 
        camera_frame_metadata_t* metadata)
    {
        REPORT_FUNCTION()
        if (!listener)
            return;
        
        switch(msg_type)
        {
            case CAMERA_MSG_RAW_IMAGE:
                if (listener->on_data_raw_image_cb)
                    listener->on_data_raw_image_cb(data->pointer(), data->size(), listener->context);                
                break;
            case CAMERA_MSG_COMPRESSED_IMAGE:
                if (listener->on_data_compressed_image_cb)
                    listener->on_data_compressed_image_cb(data->pointer(), data->size(), listener->context);
                break;                
            default:
                break;
        }

        camera->releaseRecordingFrame(data);
    }

    void postDataTimestamp(
        nsecs_t timestamp, 
        int32_t msg_type, 
        const android::sp<android::IMemory>& data)
    {
        REPORT_FUNCTION()
        (void) timestamp;
        (void) msg_type;
        (void) data;
    }
};

namespace
{
android::sp<CameraControl> camera_control_instance;
}

CameraControl* android_camera_connect_to(CameraType camera_type, CameraControlListener* listener)
{    
    REPORT_FUNCTION()
            
    int32_t camera_id;
    int32_t camera_count = camera_id = android::Camera::getNumberOfCameras();
    for (camera_id = 0; camera_id < camera_count; camera_id++)
    {
        android::CameraInfo ci;
        android::Camera::getCameraInfo(camera_id, &ci);

        if (ci.facing == camera_type)
            break;
    }

    if (camera_id == camera_count)
        return NULL;

    CameraControl* cc = new CameraControl();
    cc->listener = listener;
    cc->camera = android::Camera::connect(camera_id);

    if(cc->camera == NULL)
        return NULL;

    cc->camera_parameters = android::CameraParameters(cc->camera->getParameters());

    camera_control_instance = cc;
    cc->camera->setListener(camera_control_instance);
    cc->camera->lock();

    // TODO: Move this to a more generic component
    android::ProcessState::self()->startThreadPool();

    return cc;
}

void android_camera_dump_parameters(CameraControl* control)
{
    REPORT_FUNCTION();
    assert(control);

    printf("%s \n", control->camera->getParameters().string());
}

void android_camera_set_flash_mode(CameraControl* control, FlashMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    static const char* flash_modes[] = {
        "off",
        "auto",
        "on",
        "torch"
    };

    control->camera_parameters.set(
        android::CameraParameters::KEY_FLASH_MODE,
        flash_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_set_white_balance_mode(CameraControl* control, WhiteBalanceMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    static const char* white_balance_modes[] = {
        android::CameraParameters::WHITE_BALANCE_AUTO,
        android::CameraParameters::WHITE_BALANCE_DAYLIGHT,
        android::CameraParameters::WHITE_BALANCE_CLOUDY_DAYLIGHT,
        android::CameraParameters::WHITE_BALANCE_FLUORESCENT,
        android::CameraParameters::WHITE_BALANCE_INCANDESCENT
    };

    control->camera_parameters.set(
        android::CameraParameters::KEY_WHITE_BALANCE,
        white_balance_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_set_scene_mode(CameraControl* control, SceneMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    static const char* scene_modes[] = {
        android::CameraParameters::SCENE_MODE_AUTO,
        android::CameraParameters::SCENE_MODE_ACTION,
        android::CameraParameters::SCENE_MODE_NIGHT,
        android::CameraParameters::SCENE_MODE_PARTY,
        android::CameraParameters::SCENE_MODE_SUNSET
    };

    control->camera_parameters.set(
        android::CameraParameters::KEY_SCENE_MODE,
        scene_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_set_auto_focus_mode(CameraControl* control, AutoFocusMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    static const char* auto_focus_modes[] = {
        android::CameraParameters::FOCUS_MODE_FIXED,
        android::CameraParameters::FOCUS_MODE_CONTINUOUS_VIDEO,
        android::CameraParameters::FOCUS_MODE_AUTO,
        android::CameraParameters::FOCUS_MODE_MACRO,
        android::CameraParameters::FOCUS_MODE_CONTINUOUS_PICTURE,
        android::CameraParameters::FOCUS_MODE_INFINITY
    };

    control->camera_parameters.set(
        android::CameraParameters::KEY_FOCUS_MODE,
        auto_focus_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_set_effect_mode(CameraControl* control, EffectMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    
    static const char* effect_modes[] = 
    {
        android::CameraParameters::EFFECT_NONE,
        android::CameraParameters::EFFECT_MONO,
        android::CameraParameters::EFFECT_NEGATIVE,
        android::CameraParameters::EFFECT_SOLARIZE,
        android::CameraParameters::EFFECT_SEPIA,
        android::CameraParameters::EFFECT_POSTERIZE,
        android::CameraParameters::EFFECT_WHITEBOARD,
        android::CameraParameters::EFFECT_BLACKBOARD,
        android::CameraParameters::EFFECT_AQUA
    };

    control->camera_parameters.set(
        android::CameraParameters::KEY_EFFECT,
        effect_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_set_picture_size(CameraControl* control, PictureSize size)
{

    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    
    android::Vector<android::Size> supported_sizes;
    control->camera_parameters.getSupportedPictureSizes(supported_sizes);
    
    size_t offset = 0;
    
    switch(size)
    {
        case PICTURE_SIZE_SMALL:
            offset = supported_sizes.size() - 1;
            break;
        case PICTURE_SIZE_MEDIUM:
            offset = supported_sizes.size() / 2;
            break;
        case PICTURE_SIZE_LARGE:
            offset = 0;
            break;
    }

    android::Size selected_size = supported_sizes.itemAt(offset);
    control->camera_parameters.setPictureSize(selected_size.width, selected_size.height);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_set_display_orientation(CameraControl* control, int32_t clockwise_rotation_degree)
{
    REPORT_FUNCTION();
    assert(control);
    
    android::Mutex::Autolock al(control->guard);
    static const int32_t ignored_parameter = 0;
    control->camera->sendCommand(CAMERA_CMD_SET_DISPLAY_ORIENTATION, clockwise_rotation_degree, ignored_parameter);
}

void android_camera_set_preview_texture(CameraControl* control, SfSurface* surface)
{
    REPORT_FUNCTION()
    assert(control);
    assert(surface);
    
    android::Mutex::Autolock al(control->guard);
    control->camera->setPreviewTexture(surface->surface->getSurfaceTexture());
}

void android_camera_set_preview_surface(CameraControl* control, SfSurface* surface)
{
    REPORT_FUNCTION()
    assert(control);
    assert(surface);
    
    android::Mutex::Autolock al(control->guard);
    control->camera->setPreviewDisplay(surface->surface);
}

void android_camera_start_preview(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);

    android::Mutex::Autolock al(control->guard);
    control->camera->startPreview();
}

void android_camera_stop_preview(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    
    android::Mutex::Autolock al(control->guard);
    control->camera->stopPreview();
}

void android_camera_start_autofocus(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    
    android::Mutex::Autolock al(control->guard);
    control->camera->autoFocus();
}

void android_camera_stop_autofocus(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);

    android::Mutex::Autolock al(control->guard);
    control->camera->cancelAutoFocus();
}

void android_camera_start_zoom(CameraControl* control, int32_t zoom)
{
    REPORT_FUNCTION()
    assert(control);
    
    static const int ignored_argument = 0;

    android::Mutex::Autolock al(control->guard);
    control->camera->sendCommand(CAMERA_CMD_START_SMOOTH_ZOOM,
                                 zoom,
                                 ignored_argument);
}

void android_camera_stop_zoom(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    
    static const int ignored_argument = 0;

    android::Mutex::Autolock al(control->guard);
    control->camera->sendCommand(CAMERA_CMD_STOP_SMOOTH_ZOOM,
                                 ignored_argument,
                                 ignored_argument);
}

void android_camera_take_snapshot(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    android::Mutex::Autolock al(control->guard);
    control->camera->takePicture(CAMERA_MSG_SHUTTER | CAMERA_MSG_COMPRESSED_IMAGE);
}
