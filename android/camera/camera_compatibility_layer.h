/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#ifndef CAMERA_COMPATIBILITY_LAYER_H_
#define CAMERA_COMPATIBILITY_LAYER_H_

#ifdef __cplusplus
extern "C" {
#endif
    
    #include <stdint.h>

    // Forward declarations
    struct SfSurface;

    typedef enum
    {
        FRONT_FACING_CAMERA_TYPE,
        BACK_FACING_CAMERA_TYPE
    } CameraType;

    struct CameraControl;
    
    struct CameraControlListener
    {
        typedef void (*on_msg_error)(void* context);
        typedef void (*on_msg_shutter)(void* context);
        typedef void (*on_msg_focus)(void* context);
        typedef void (*on_msg_zoom)(void* context);

        typedef void (*on_data_raw_image)(void* data, uint32_t data_size, void* context);
        typedef void (*on_data_compressed_image)(void* data, uint32_t data_size, void* context);
        
        on_msg_error on_msg_error_cb;
        on_msg_shutter on_msg_shutter_cb;
        on_msg_focus on_msg_focus_cb;
        on_msg_zoom on_msg_zoom_cb;

        on_data_raw_image on_data_raw_image_cb;
        on_data_compressed_image on_data_compressed_image_cb;
        
        void* context;
    };

    CameraControl* android_camera_connect_to(CameraType camera_type, CameraControlListener* listener);
    
    void android_camera_set_preview_surface(CameraControl* control, SfSurface* surface);

    void android_camera_start_preview(CameraControl* control);

    void android_camera_stop_preview(CameraControl* control);
    
    void android_camera_start_autofocus(CameraControl* control);
    
    void android_camera_stop_autofocus(CameraControl* control);

    void android_camera_start_zoom(CameraControl* control, float zoom);

    void android_camera_stop_zoom(CameraControl* control);
    
    void android_camera_take_snapshot(CameraControl* control);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_COMPATIBILITY_LAYER_H_
