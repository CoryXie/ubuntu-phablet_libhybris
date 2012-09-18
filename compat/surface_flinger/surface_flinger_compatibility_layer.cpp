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

#include "surface_flinger_compatibility_layer.h"
#include "surface_flinger_compatibility_layer_internal.h"

#include <utils/misc.h>

#include <surfaceflinger/SurfaceComposerClient.h>
#include <ui/PixelFormat.h>
#include <ui/Region.h>
#include <ui/Rect.h>

#include <cassert>
#include <cstdio>

namespace
{
void report_failed_to_allocate_surface_flinger_composer_client_on_creation()
{
    printf("Problem allocating an object of type SurfaceComposerClient during client creation");
}

void report_failed_to_get_egl_default_display_on_creation()
{
    printf("Problem accessing default egl display during client creation");
}

void report_failed_to_initialize_egl_on_creation()
{
    printf("Problem initializing egl during client creation");
}

void report_failed_to_choose_egl_config_on_creation()
{
    printf("Problem choosing egl config on creation");
}

void report_surface_control_is_null_during_creation()
{
    printf("Could not acquire surface control object during surface creation");
}

void report_surface_is_null_during_creation()
{
    printf("Could not acquire surface from surface control during surface creation");
}

}

size_t sf_get_number_of_displays()
{
    return android::SurfaceComposerClient::getNumberOfDisplays();
}

size_t sf_get_display_width(size_t display_id)
{
    return android::SurfaceComposerClient::getDisplayWidth(display_id);
}

size_t sf_get_display_height(size_t display_id)
{
    return android::SurfaceComposerClient::getDisplayHeight(display_id);
}

SfClient* sf_client_create()
{
    
    SfClient* client = new SfClient();
        
    client->client = new android::SurfaceComposerClient();
    if (client->client == NULL) 
    {
        report_failed_to_allocate_surface_flinger_composer_client_on_creation();
        delete client;
        return NULL;
    }
    
    client->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (client->egl_display == EGL_NO_DISPLAY)
    {
        report_failed_to_get_egl_default_display_on_creation();
        delete client;
        return NULL;
    }
    
    int major, minor;
    int rc = eglInitialize(client->egl_display, &major, &minor);
    if (rc == EGL_FALSE) {
        report_failed_to_initialize_egl_on_creation();
        delete client;
        return NULL;
    }

    EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE };

    int n;
    if (eglChooseConfig(
            client->egl_display, 
            attribs, 
            &client->egl_config, 
            1, 
            &n) == EGL_FALSE) {
        report_failed_to_choose_egl_config_on_creation();
        delete client;
        return NULL;
    }

    EGLint context_attribs[] = { 
        EGL_CONTEXT_CLIENT_VERSION, 2, 
        EGL_NONE };
    
    client->egl_context = eglCreateContext(
        client->egl_display, 
        client->egl_config, 
        EGL_NO_CONTEXT, 
        context_attribs);
    
    return client;
}

EGLDisplay sf_client_get_egl_display(SfClient* client)
{
    assert(client);
    return client->egl_display;
}

EGLConfig sf_client_get_egl_config(SfClient* client)
{
    assert(client);
    return client->egl_config;
}

void sf_client_begin_transaction(SfClient* client)
{
    assert(client);
    client->client->openGlobalTransaction();
}

void sf_client_end_transaction(SfClient* client)
{
    assert(client);
    client->client->closeGlobalTransaction();
}

SfSurface* sf_surface_create(SfClient* client, SfSurfaceCreationParameters* params)
{
    assert(client);
    assert(params);
    
    SfSurface* surface = new SfSurface();
    surface->client = client;
    surface->surface_control = surface->client->client->createSurface(
        android::String8(params->name), 
        0, 
        params->w, 
        params->h, 
        android::PIXEL_FORMAT_RGBA_8888,
        0x300);

    if (surface->surface_control == NULL)
    {
        report_surface_control_is_null_during_creation();
        delete(surface);
        return NULL;
    }

    surface->surface = surface->surface_control->getSurface();

    if (surface->surface == NULL)
    {
        report_surface_is_null_during_creation();
        delete(surface);
        return NULL;
    }

    sf_client_begin_transaction(client);
    {
        surface->surface_control->setPosition(params->x, params->y);
        surface->surface_control->setLayer(params->layer);
        surface->surface_control->setAlpha(params->alpha);
    } 
    sf_client_end_transaction(client);

    if (params->create_egl_window_surface)
    {
        android::sp<ANativeWindow> anw(surface->surface);
        surface->egl_surface = eglCreateWindowSurface(
            surface->client->egl_display,
            surface->client->egl_config,
            anw.get(),
            NULL);
    }
    
    return surface;
}

EGLSurface sf_surface_get_egl_surface(SfSurface* surface)
{
    assert(surface);
    return surface->egl_surface;
}

void sf_surface_make_current(SfSurface* surface)
{
    assert(surface);
    eglMakeCurrent(
        surface->client->egl_display,
        surface->egl_surface,
        surface->egl_surface,
        surface->client->egl_context);
}

void sf_surface_move_to(SfSurface* surface, int x, int y)
{
    assert(surface);
    surface->surface_control->setPosition(x, y);
}

void sf_surface_set_layer(SfSurface* surface, int layer)
{
    assert(surface);
    surface->surface_control->setLayer(layer);
}

void sf_surface_set_alpha(SfSurface* surface, float alpha)
{
    assert(surface);
    surface->surface_control->setAlpha(alpha);
}