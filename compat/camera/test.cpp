#include "camera_compatibility_layer.h"

#include <input_stack_compatibility_layer.h>
#include <surface_flinger_compatibility_layer.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstring>

int shot_counter = 1;
int32_t current_zoom_level = 1;

EffectMode next_effect()
{
    static EffectMode current_effect = EFFECT_MODE_NONE;

    EffectMode next = current_effect;
    
    switch(current_effect)
    {
        case EFFECT_MODE_NONE:
            next = EFFECT_MODE_MONO;
            break;
        case EFFECT_MODE_MONO:
            next = EFFECT_MODE_NEGATIVE;
            break;
        case EFFECT_MODE_NEGATIVE:
            next = EFFECT_MODE_SOLARIZE;
            break;
        case EFFECT_MODE_SOLARIZE:
            next = EFFECT_MODE_SEPIA;
            break;
        case EFFECT_MODE_SEPIA:
            next = EFFECT_MODE_POSTERIZE;
            break;
        case EFFECT_MODE_POSTERIZE:
            next = EFFECT_MODE_WHITEBOARD;
            break;
        case EFFECT_MODE_WHITEBOARD:
            next = EFFECT_MODE_BLACKBOARD;
            break;
        case EFFECT_MODE_BLACKBOARD:
            next = EFFECT_AQUA;
            break;
        case EFFECT_AQUA:
            next = EFFECT_MODE_NONE;
            break;
    }

    current_effect = next;
    return next;
}

void error_msg_cb(void* context)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

void shutter_msg_cb(void* context)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

void zoom_msg_cb(void* context, int32_t new_zoom_level)
{
    printf("%s \n", __PRETTY_FUNCTION__);
    current_zoom_level = new_zoom_level;
}

void autofocus_msg_cb(void* context)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

void raw_data_cb(void* data, uint32_t data_size, void* context)
{
    printf("%s: %d \n", __PRETTY_FUNCTION__, data_size);
}

void jpeg_data_cb(void* data, uint32_t data_size, void* context)
{
    printf("%s: %d \n", __PRETTY_FUNCTION__, data_size);

    char fn[256];
    sprintf(fn, "/data/shot_%d.jpeg", shot_counter);
    int fd = open(fn, O_RDWR | O_CREAT);
    write(fd, data, data_size);
    close(fd);
    shot_counter++;

    CameraControl* cc = static_cast<CameraControl*>(context);
    android_camera_start_preview(cc);
}

void on_new_input_event(Event* event, void* context)
{
    assert(context);

    if (event->type == KEY_EVENT_TYPE)
    {
        printf("We have got a key event: %d \n", event->details.key.key_code);

        CameraControl* cc = static_cast<CameraControl*>(context);
        
        switch(event->details.key.key_code)
        {
            case 24:
                printf("\tZooming in now.\n");
                android_camera_start_zoom(cc, current_zoom_level+1);
                break;
            case 25:
                printf("\tZooming out now.\n");
                android_camera_start_zoom(cc, current_zoom_level-1);
                break;
            case 26:
                printf("\tTaking a photo now.\n");
                android_camera_take_snapshot(cc);
                break;
            case 79:
                printf("\tSwitching effect.\n");
                android_camera_set_effect_mode(cc, next_effect());
                
        }
    } else if (event->type == MOTION_EVENT_TYPE)
    {
        CameraControl* cc = static_cast<CameraControl*>(context);
    }
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

    static const size_t primary_display = 0;

    SfSurfaceCreationParameters params =
    {
        0,
        0,
        sf_get_display_width(primary_display),
        sf_get_display_height(primary_display),
        -1, //PIXEL_FORMAT_RGBA_8888,
        15000,
        0.5f,
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
    listener.on_msg_error_cb = error_msg_cb;
    listener.on_msg_shutter_cb = shutter_msg_cb;
    listener.on_msg_focus_cb = autofocus_msg_cb;
    listener.on_msg_zoom_cb = zoom_msg_cb;

    listener.on_data_raw_image_cb = raw_data_cb;
    listener.on_data_compressed_image_cb = jpeg_data_cb;
    CameraControl* cc = android_camera_connect_to(FRONT_FACING_CAMERA_TYPE,
                                                  &listener);
    if (cc == NULL)
    {
        printf("Problem connecting to camera");
        return 1;
    }

    listener.context = cc;

    AndroidEventListener event_listener;
    event_listener.on_new_event = on_new_input_event;
    event_listener.context = cc;
    
    InputStackConfiguration input_configuration = { true, 25000 };
    
    android_input_stack_initialize(&event_listener, &input_configuration);
    android_input_stack_start();
    
    android_camera_dump_parameters(cc);

    ClientWithSurface cs = client_with_surface();

    if (!cs.surface)
    {
        printf("Problem acquiring surface for preview");
        return 1;
    }

    EGLDisplay disp = sf_client_get_egl_display(cs.client);
    EGLSurface surface = sf_surface_get_egl_surface(cs.surface);
    
    android_camera_set_preview_texture(cc, cs.surface);
    android_camera_set_display_orientation(cc, 90);
    android_camera_set_picture_size(cc, PICTURE_SIZE_LARGE);
    android_camera_set_effect_mode(cc, EFFECT_MODE_SEPIA);
    android_camera_set_flash_mode(cc, FLASH_MODE_AUTO);
    android_camera_set_auto_focus_mode(cc, AUTO_FOCUS_MODE_CONTINUOUS_PICTURE);
    android_camera_start_preview(cc);
    
    while(true)
    {
    }
}
