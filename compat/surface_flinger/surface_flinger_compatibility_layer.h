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

#ifndef SURFACE_FLINGER_COMPATIBILITY_LAYER_H_
#define SURFACE_FLINGER_COMPATIBILITY_LAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

    struct SfClient;
    struct SfSurface;

    enum
    {
        SURFACE_FLINGER_DEFAULT_DISPLAY_ID = 0
    };

    void sf_blank(size_t display_id);
    void sf_unblank(size_t display_id);

    size_t sf_get_display_width(size_t display_id);
    size_t sf_get_display_height(size_t display_id);

    // The egl_support parameter disables the use of EGL inside the
    // library. sf_client_create() enables the use of EGL by default. When
    // disabled, the functions sf_client_get_egl_display(),
    // sf_client_get_egl_config(), sf_surface_get_egl_surface(),
    // sf_surface_make_current() and the create_egl_window_surface feature are
    // not supported anymore.
    SfClient* sf_client_create_full(bool egl_support);
    SfClient* sf_client_create();

    EGLDisplay sf_client_get_egl_display(SfClient*);
    EGLConfig sf_client_get_egl_config(SfClient* client);
    void sf_client_begin_transaction(SfClient*);
    void sf_client_end_transaction(SfClient*);

    typedef struct
    {
        int x;
        int y;
        int w;
        int h;
        int format;
        int layer;
        float alpha;
        bool create_egl_window_surface;
        const char* name;
    } SfSurfaceCreationParameters;

    SfSurface* sf_surface_create(SfClient* client, SfSurfaceCreationParameters* params);
    EGLSurface sf_surface_get_egl_surface(SfSurface*);
    EGLNativeWindowType sf_surface_get_egl_native_window(SfSurface*);
    void sf_surface_make_current(SfSurface* surface);

    void sf_surface_move_to(SfSurface* surface, int x, int y);
    void sf_surface_set_size(SfSurface* surface, int w, int h);
    void sf_surface_set_layer(SfSurface* surface, int layer);
    void sf_surface_set_alpha(SfSurface* surface, float alpha);

#ifdef __cplusplus
}
#endif

#endif // SURFACE_FLINGER_COMPATIBILITY_LAYER_H_
