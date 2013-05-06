/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#include <camera_compatibility_layer.h>
#include <camera_compatibility_layer_capabilities.h>
#include <recorder_compatibility_layer.h>

#include <assert.h>
#include <dlfcn.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void *android_dlopen(const char *filename, int flag);
extern void *android_dlsym(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

namespace
{

struct CameraBridge
{
    static const char* path_to_library()
    {
        return "/system/lib/libcamera_compat_layer.so";
    }
    
    CameraBridge() : libcamera_handle(android_dlopen(path_to_library(), RTLD_LAZY))
    {
        assert(libcamera_handle && "Error loading camera library from");
    }

    ~CameraBridge()
    {
        // TODO android_dlclose(libcamera_handle);
    }

    void* resolve_symbol(const char* symbol) const
    {
        return android_dlsym(libcamera_handle, symbol);
    }
    
    void* libcamera_handle;
};

static CameraBridge camera_bridge;

}

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/
/*********** Implementation starts here *******************/
/**********************************************************/

#define CAMERA_DLSYM(fptr, sym) if (*(fptr) == NULL) { *(fptr) = (void *) camera_bridge.resolve_symbol(sym); }
    
#define IMPLEMENT_FUNCTION0(return_type, symbol)  \
    return_type symbol()                          \
    {                                             \
        static return_type (*f)() = NULL;         \
        CAMERA_DLSYM(&f, #symbol);                \
        return f();}
    
#define IMPLEMENT_FUNCTION1(return_type, symbol, arg1) \
    return_type symbol(arg1 _1)                        \
    {                                                  \
        static return_type (*f)(arg1) = NULL;          \
        CAMERA_DLSYM(&f, #symbol);                     \
        return f(_1); }

#define IMPLEMENT_VOID_FUNCTION1(symbol, arg1)               \
    void symbol(arg1 _1)                                     \
    {                                                        \
        static void (*f)(arg1) = NULL;                       \
        CAMERA_DLSYM(&f, #symbol);                           \
        f(_1); }

#define IMPLEMENT_FUNCTION2(return_type, symbol, arg1, arg2)    \
    return_type symbol(arg1 _1, arg2 _2)                        \
    {                                                           \
        static return_type (*f)(arg1, arg2) = NULL;             \
        CAMERA_DLSYM(&f, #symbol);                              \
        return f(_1, _2); }

#define IMPLEMENT_VOID_FUNCTION2(symbol, arg1, arg2)            \
    void symbol(arg1 _1, arg2 _2)                               \
    {                                                           \
        static void (*f)(arg1, arg2) = NULL;                    \
        CAMERA_DLSYM(&f, #symbol);                              \
        f(_1, _2); }

#define IMPLEMENT_FUNCTION3(return_type, symbol, arg1, arg2, arg3) \
    return_type symbol(arg1 _1, arg2 _2, arg3 _3)                        \
    {                                                           \
        static return_type (*f)(arg1, arg2, arg3) = NULL;             \
        CAMERA_DLSYM(&f, #symbol);                              \
        return f(_1, _2, _3); }

#define IMPLEMENT_VOID_FUNCTION3(symbol, arg1, arg2, arg3)      \
    void symbol(arg1 _1, arg2 _2, arg3 _3)                      \
    {                                                           \
        static void (*f)(arg1, arg2, arg3) = NULL;              \
        CAMERA_DLSYM(&f, #symbol);                              \
        f(_1, _2, _3); }


IMPLEMENT_FUNCTION0(int, android_camera_get_number_of_devices);
IMPLEMENT_FUNCTION2(CameraControl*, android_camera_connect_to, CameraType, CameraControlListener*);
IMPLEMENT_VOID_FUNCTION1(android_camera_disconnect, CameraControl*);
IMPLEMENT_FUNCTION1(int, android_camera_lock, CameraControl*);
IMPLEMENT_FUNCTION1(int, android_camera_unlock, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_delete, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_dump_parameters, CameraControl*);

// Setters
IMPLEMENT_VOID_FUNCTION2(android_camera_set_effect_mode, CameraControl*, EffectMode);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_flash_mode, CameraControl*, FlashMode);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_white_balance_mode, CameraControl*, WhiteBalanceMode);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_scene_mode, CameraControl*, SceneMode);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_auto_focus_mode, CameraControl*, AutoFocusMode);
IMPLEMENT_VOID_FUNCTION3(android_camera_set_picture_size, CameraControl*, int, int);
IMPLEMENT_VOID_FUNCTION3(android_camera_set_preview_size, CameraControl*, int, int);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_display_orientation, CameraControl*, int32_t);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_preview_texture, CameraControl*, int);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_preview_surface, CameraControl*, SfSurface*);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_focus_region, CameraControl*, FocusRegion*);
IMPLEMENT_VOID_FUNCTION1(android_camera_reset_focus_region, CameraControl*);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_preview_fps, CameraControl*, int);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_rotation, CameraControl*, int);
IMPLEMENT_VOID_FUNCTION3(android_camera_set_video_size, CameraControl*, int, int);
// Getters
IMPLEMENT_VOID_FUNCTION2(android_camera_get_effect_mode, CameraControl*, EffectMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_flash_mode, CameraControl*, FlashMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_white_balance_mode, CameraControl*, WhiteBalanceMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_scene_mode, CameraControl*, SceneMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_auto_focus_mode, CameraControl*, AutoFocusMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_max_zoom, CameraControl*, int*);
IMPLEMENT_VOID_FUNCTION3(android_camera_get_picture_size, CameraControl*, int*, int*);
IMPLEMENT_VOID_FUNCTION3(android_camera_get_preview_size, CameraControl*, int*, int*);
IMPLEMENT_VOID_FUNCTION3(android_camera_get_preview_fps_range, CameraControl*, int*, int*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_preview_fps, CameraControl*, int*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_preview_texture_transformation, CameraControl*, float*);
IMPLEMENT_VOID_FUNCTION3(android_camera_get_video_size, CameraControl*, int*, int*);

// Enumerators
IMPLEMENT_VOID_FUNCTION3(android_camera_enumerate_supported_picture_sizes, CameraControl*, size_callback, void*);
IMPLEMENT_VOID_FUNCTION3(android_camera_enumerate_supported_preview_sizes, CameraControl*, size_callback, void*);
IMPLEMENT_VOID_FUNCTION3(android_camera_enumerate_supported_video_sizes, CameraControl*, size_callback, void*);

IMPLEMENT_VOID_FUNCTION1(android_camera_update_preview_texture, CameraControl*);

IMPLEMENT_VOID_FUNCTION1(android_camera_start_preview, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_stop_preview, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_start_autofocus, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_stop_autofocus, CameraControl*);

IMPLEMENT_VOID_FUNCTION2(android_camera_start_zoom, CameraControl*, int32_t);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_zoom, CameraControl*, int32_t);
IMPLEMENT_VOID_FUNCTION1(android_camera_stop_zoom, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_take_snapshot, CameraControl*);

// Recorder
IMPLEMENT_FUNCTION0(MediaRecorderWrapper*, android_media_new_recorder);
IMPLEMENT_FUNCTION1(int, android_recorder_initCheck, MediaRecorderWrapper*);
IMPLEMENT_FUNCTION2(int, android_recorder_setCamera, MediaRecorderWrapper*, CameraControl*);
IMPLEMENT_FUNCTION2(int, android_recorder_setVideoSource, MediaRecorderWrapper*, int);
IMPLEMENT_FUNCTION2(int, android_recorder_setAudioSource, MediaRecorderWrapper*, int);
IMPLEMENT_FUNCTION2(int, android_recorder_setOutputFormat, MediaRecorderWrapper*, int);
IMPLEMENT_FUNCTION2(int, android_recorder_setVideoEncoder, MediaRecorderWrapper*, int);
IMPLEMENT_FUNCTION2(int, android_recorder_setAudioEncoder, MediaRecorderWrapper*, int);
IMPLEMENT_FUNCTION2(int, android_recorder_setOutputFile, MediaRecorderWrapper*, int);
IMPLEMENT_FUNCTION3(int, android_recorder_setVideoSize, MediaRecorderWrapper*, int, int);
IMPLEMENT_FUNCTION2(int, android_recorder_setVideoFrameRate, MediaRecorderWrapper*, int);
IMPLEMENT_FUNCTION1(int, android_recorder_start, MediaRecorderWrapper*);
IMPLEMENT_FUNCTION1(int, android_recorder_stop, MediaRecorderWrapper*);
IMPLEMENT_FUNCTION1(int, android_recorder_prepare, MediaRecorderWrapper*);
IMPLEMENT_FUNCTION1(int, android_recorder_reset, MediaRecorderWrapper*);

#ifdef __cplusplus
}
#endif
