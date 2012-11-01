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

    typedef void (*on_msg_set_video_size)(int height, int width, void *context);

    class Player;

    // ----- Start of C API ----- //

    void android_media_set_video_size_cb(on_msg_set_video_size cb);

    Player *android_media_new_player();
    int android_media_set_data_source(const char* url);
    int android_media_set_preview_texture(int texture_id);
    void android_media_update_surface_texture();
    void android_media_surface_texture_get_transformation_matrix(GLfloat* matrix);
    int android_media_play();
    int android_media_pause();
    int android_media_stop();
    bool android_media_is_playing();

    int android_media_seek_to(int msec);
    int android_media_get_current_position(int *msec);
    int android_media_get_duration(int *msec);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_COMPATIBILITY_LAYER_H_
