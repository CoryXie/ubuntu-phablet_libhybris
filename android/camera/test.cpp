#include "camera_compatibility_layer.h"

#include <surface_flinger_compatibility_layer.h>
#include <cstdio>
#include <string.h>

void msg_cb(void* context)
{
    (void) context;
    printf("%s \n", __PRETTY_FUNCTION__);
}

void data_cb(void* data, uint32_t data_size, void* context)
{
    (void) context;
    (void) data;
    (void) data_size;
    printf("%s \n", __PRETTY_FUNCTION__);
}

struct ClientWithSurface
{
    SfClient* client;
    SfSurface* surface;
};

ClientWithSurface client_with_surface()
{
    ClientWithSurface cs = ClientWithSurface();

    cs.client = sf_client_create();

    if (!cs.client)
    {
        printf("Problem creating client ... aborting now.");
        return cs;
    }

    SfSurfaceCreationParameters params =
    {
        200,
        200,
        500,
        500,
        -1, //PIXEL_FORMAT_RGBA_8888,
        15000,
        1.0f,
        false, // Do not associate surface with egl, will be done by camera HAL
        "CameraCompatLayerTestSurface"
    };

    cs.surface = sf_surface_create(cs.client, &params);

    if (!cs.surface)
    {
        printf("Problem creating surface ... aborting now.");
        return cs;
    }

    sf_surface_make_current(cs.surface);

    return cs;
}

int main(int argc, char** argv)
{
    CameraControlListener listener;
    memset(&listener, 0, sizeof(listener));
    listener.on_msg_error_cb = msg_cb;
    listener.on_msg_shutter_cb = msg_cb;
    listener.on_msg_focus_cb = msg_cb;
    listener.on_msg_zoom_cb = msg_cb;


    listener.on_data_raw_image_cb = data_cb;
    listener.on_data_compressed_image_cb = data_cb;

    CameraControl* cc = android_camera_connect_to(BACK_FACING_CAMERA_TYPE,
                                                  &listener);

    if (cc == NULL)
    {
        printf("Problem connecting to camera");
        return 1;
    }

    ClientWithSurface cs = client_with_surface();

    if (!cs.surface)
    {
        printf("Problem acquiring surface for preview");
        return 1;
    }

    EGLDisplay disp = sf_client_get_egl_display(cs.client);
    EGLSurface surface = sf_surface_get_egl_surface(cs.surface);
    
    android_camera_set_preview_surface(cc, cs.surface);
    android_camera_start_preview(cc);

    while(true)
    {
    }
}
