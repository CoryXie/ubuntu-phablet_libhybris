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

    typedef enum
    {
        FLASH_MODE_OFF,
        FLASH_MODE_AUTO,
        FLASH_MODE_ON,
        FLASH_MODE_TORCH
    } FlashMode;

    typedef enum
    {
        WHITE_BALANCE_MODE_AUTO,
        WHITE_BALANCE_MODE_DAYLIGHT,
        WHITE_BALANCE_MODE_CLOUDY_DAYLIGHT,
        WHITE_BALANCE_MODE_FLUORESCENT,
        WHITE_BALANCE_MODE_INCANDESCENT        
    } WhiteBalanceMode;

    typedef enum
    {
        SCENE_MODE_AUTO,
        SCENE_MODE_ACTION,
        SCENE_MODE_NIGHT,
        SCENE_MODE_PARTY,
        SCENE_MODE_SUNSET
    } SceneMode;

    typedef enum
    {
        AUTO_FOCUS_MODE_OFF,
        AUTO_FOCUS_MODE_CONTINUOUS_VIDEO,
        AUTO_FOCUS_MODE_AUTO,
        AUTO_FOCUS_MODE_MACRO,
        AUTO_FOCUS_MODE_CONTINUOUS_PICTURE,
        AUTO_FOCUS_MODE_INFINITY
    } AutoFocusMode;

    typedef enum
    {
        EFFECT_MODE_NONE,
        EFFECT_MODE_MONO,
        EFFECT_MODE_NEGATIVE,
        EFFECT_MODE_SOLARIZE,
        EFFECT_MODE_SEPIA,
        EFFECT_MODE_POSTERIZE,
        EFFECT_MODE_WHITEBOARD,
        EFFECT_MODE_BLACKBOARD,
        EFFECT_AQUA
    } EffectMode;

    typedef enum
    {
        PICTURE_SIZE_SMALL,
        PICTURE_SIZE_MEDIUM,
        PICTURE_SIZE_LARGE
    } PictureSize;

    struct CameraControl;
    
    struct CameraControlListener
    {
        typedef void (*on_msg_error)(void* context);
        typedef void (*on_msg_shutter)(void* context);
        typedef void (*on_msg_focus)(void* context);
        typedef void (*on_msg_zoom)(void* context, int32_t new_zoom_level);

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
    
    void android_camera_dump_parameters(CameraControl* control);
    
    void android_camera_set_effect_mode(CameraControl* control, EffectMode mode);
    void android_camera_set_flash_mode(CameraControl* control, FlashMode mode);
    void android_camera_set_white_balance_mode(CameraControl* control, WhiteBalanceMode mode);
    void android_camera_set_scene_mode(CameraControl* control, SceneMode mode);
    void android_camera_set_auto_focus_mode(CameraControl* control, AutoFocusMode mode);
    void android_camera_set_picture_size(CameraControl* control, PictureSize size);
    
    void android_camera_set_display_orientation(CameraControl* control, int32_t clockwise_rotation_degree);

    void android_camera_set_preview_texture(CameraControl* control, SfSurface* surface);

    void android_camera_set_preview_surface(CameraControl* control, SfSurface* surface);

    void android_camera_start_preview(CameraControl* control);

    void android_camera_stop_preview(CameraControl* control);
    
    void android_camera_start_autofocus(CameraControl* control);
    
    void android_camera_stop_autofocus(CameraControl* control);

    void android_camera_start_zoom(CameraControl* control, int32_t zoom);

    void android_camera_stop_zoom(CameraControl* control);
    
    void android_camera_take_snapshot(CameraControl* control);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_COMPATIBILITY_LAYER_H_
