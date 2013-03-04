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
 * Authored by: Jim Hodapp <jim.hodapp@canonical.com>
 */

#ifndef MEDIA_COMPATIBILITY_LAYER_H_
#define MEDIA_COMPATIBILITY_LAYER_H_

#include <GLES2/gl2.h>

#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

    // Callback types
    typedef void (*on_msg_set_video_size)(int height, int width, void *context);
    typedef void (*on_video_texture_needs_update)(void *context);
    typedef void (*on_msg_error)(void *context);
    typedef void (*on_playback_complete)(void *context);
    typedef void (*on_media_prepared)(void *context);

    struct MediaPlayerWrapper;

    // ----- Start of C API ----- //

    // Callback setters
    void android_media_set_video_size_cb(MediaPlayerWrapper *mp, on_msg_set_video_size cb, void *context);
    void android_media_set_video_texture_needs_update_cb(MediaPlayerWrapper *mp, on_video_texture_needs_update cb, void *context);
    void android_media_set_error_cb(MediaPlayerWrapper *mp, on_msg_error cb, void *context);
    void android_media_set_playback_complete_cb(MediaPlayerWrapper *mp, on_playback_complete cb, void *context);
    void android_media_set_media_prepared_cb(MediaPlayerWrapper *mp, on_media_prepared cb, void *context);

    // Main player control API
    MediaPlayerWrapper *android_media_new_player();
    int android_media_set_data_source(MediaPlayerWrapper *mp, const char* url);
    int android_media_set_preview_texture(MediaPlayerWrapper *mp, int texture_id);
    void android_media_update_surface_texture(MediaPlayerWrapper *mp);
    void android_media_surface_texture_get_transformation_matrix(MediaPlayerWrapper *mp, GLfloat*matrix);
    int android_media_play(MediaPlayerWrapper *mp);
    int android_media_pause(MediaPlayerWrapper *mp);
    int android_media_stop(MediaPlayerWrapper *mp);
    bool android_media_is_playing(MediaPlayerWrapper *mp);

    int android_media_seek_to(MediaPlayerWrapper *mp, int msec);
    int android_media_get_current_position(MediaPlayerWrapper *mp, int *msec);
    int android_media_get_duration(MediaPlayerWrapper *mp, int *msec);

    int android_media_get_volume(MediaPlayerWrapper *mp, int *volume);
    int android_media_set_volume(MediaPlayerWrapper *mp, int volume);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_COMPATIBILITY_LAYER_H_
