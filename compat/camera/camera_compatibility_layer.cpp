#include "camera_compatibility_layer.h"

#include <surface_flinger_compatibility_layer_internal.h>

#include <camera/Camera.h>

#define REPORT_FUNCTION() printf("%s \n", __PRETTY_FUNCTION__);

struct CameraControl : public android::CameraListener
{
    CameraControlListener* listener;
    android::sp<android::Camera> camera;

    void notify(int32_t msg_type, int32_t ext1, int32_t ext2)
    {
        REPORT_FUNCTION()

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
                    listener->on_msg_zoom_cb(listener->context);
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
    cc->camera->setListener(android::sp<android::CameraListener>(cc));
    
    return cc;
}

void android_camera_set_preview_surface(CameraControl* control, SfSurface* surface)
{
    REPORT_FUNCTION()
    assert(control);
    assert(surface);
    
    control->camera->setPreviewDisplay(surface->surface);
}

void android_camera_start_preview(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);

    control->camera->lock();
    control->camera->startPreview();
    control->camera->unlock();
}

void android_camera_stop_preview(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    
    control->camera->lock();
    control->camera->stopPreview();
    control->camera->unlock();
}

void android_camera_start_autofocus(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    
    control->camera->lock();
    control->camera->autoFocus();
    control->camera->unlock();
}

void android_camera_stop_autofocus(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    
    control->camera->lock();
    control->camera->cancelAutoFocus();
    control->camera->unlock();
}

void android_camera_start_zoom(CameraControl* control, int32_t zoom)
{
    REPORT_FUNCTION()
    assert(control);
    
    static const int ignored_argument = 0;

    control->camera->lock();
    control->camera->sendCommand(CAMERA_CMD_START_SMOOTH_ZOOM,
                                 zoom,
                                 ignored_argument);
    control->camera->unlock();
}

void android_camera_stop_zoom(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    
    static const int ignored_argument = 0;

    control->camera->lock();
    control->camera->sendCommand(CAMERA_CMD_STOP_SMOOTH_ZOOM,
                                 ignored_argument,
                                 ignored_argument);
    control->camera->unlock();
}

void android_camera_take_snapshot(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    control->camera->lock();
    control->camera->takePicture(CAMERA_MSG_RAW_IMAGE | CAMERA_MSG_COMPRESSED_IMAGE);
    control->camera->unlock();
}
