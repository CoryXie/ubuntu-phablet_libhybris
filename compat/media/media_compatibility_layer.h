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

#include "media_compatibility_layer_capabilities.h"
#include <media/MediaPlayerInterface.h>

#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

    class Player;

    // ----- Start of C API ----- //

    Player *android_media_new_player();
    android::status_t android_media_set_data_source(const char* url);
    android::status_t android_media_set_preview_texture(int texture_id);
    void android_media_update_surface_texture();
    android::status_t android_media_play();
    android::status_t android_media_pause();
    android::status_t android_media_stop();
    bool android_media_is_playing();

#ifdef __cplusplus
}
#endif

#endif // CAMERA_COMPATIBILITY_LAYER_H_
