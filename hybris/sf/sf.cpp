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
 * Authored by: Michael Frey <michael.frey@canonical.com>
 */

#include <EGL/egl.h>
#include <dlfcn.h>
#include <stddef.h>

#include <surface_flinger_compatibility_layer.h>

#ifdef __cplusplus
extern "C" {
#endif


extern void *android_dlopen(const char *filename, int flag);
extern void *android_dlsym(void *handle, const char *symbol);

static SfSurface* (*_sf_surface_create)(SfClient* client, SfSurfaceCreationParameters* params) = NULL;

static EGLSurface (*_sf_surface_get_egl_surface)(SfSurface*) = NULL;
static EGLNativeWindowType (*_sf_surface_get_egl_native_window)(SfSurface*) = NULL;
static void (*_sf_surface_make_current)(SfSurface* surface) = NULL;

static void (*_sf_surface_move_to)(SfSurface* surface, int x, int y) = NULL;
static void (*_sf_surface_set_layer)(SfSurface* surface, int layer) = NULL;
static void (*_sf_surface_set_alpha)(SfSurface* surface, float alpha) = NULL;

static SfClient* (*_sf_client_create)(void) = NULL;
static EGLDisplay (*_sf_client_get_egl_display)(SfClient* client) = NULL;
static EGLConfig (*_sf_client_get_egl_config)(SfClient* client) = NULL;
static void (*_sf_client_begin_transaction)(SfClient* client) = NULL;
static void (*_sf_client_end_transaction)(SfClient* client) = NULL;

static void *_libsf = NULL;

static void _init_androidsf()
{
 _libsf = (void *) android_dlopen("/system/lib/libsf_compat_layer.so", RTLD_LAZY);
}

#define SF_DLSYM(fptr, sym) do { if (_libsf == NULL) { _init_androidsf(); }; if (*(fptr) == NULL) { *(fptr) = (void *) android_dlsym(_libsf, sym); } } while (0)

SfSurface* sf_surface_create(SfClient* client, SfSurfaceCreationParameters* params)
{
  SF_DLSYM(&_sf_surface_create, "sf_surface_create");
  return (*_sf_surface_create)(client,params);
}

EGLSurface sf_surface_get_egl_surface(SfSurface* surface)
{
  SF_DLSYM(&_sf_surface_get_egl_surface, "sf_surface_get_egl_surface");
  return (*_sf_surface_get_egl_surface)(surface);
}

EGLNativeWindowType sf_surface_get_egl_native_window(SfSurface* surface)
{
  SF_DLSYM(&_sf_surface_get_egl_native_window, "sf_surface_get_egl_native_window");
  return (*_sf_surface_get_egl_native_window)(surface);
}

void sf_surface_make_current(SfSurface* surface)
{
  SF_DLSYM(&_sf_surface_make_current, "sf_surface_make_current");
  return (*_sf_surface_make_current)(surface);
}

void sf_surface_move_to(SfSurface* surface, int x, int y)
{
  SF_DLSYM(&_sf_surface_move_to, "sf_surface_move_to");
  return (*_sf_surface_move_to)(surface, x, y);
}

void sf_surface_set_layer(SfSurface* surface, int layer)
{
  SF_DLSYM(&_sf_surface_set_layer, "sf_surface_set_layer");
  return (*_sf_surface_set_layer)(surface, layer);
}

void sf_surface_set_alpha(SfSurface* surface, float alpha)
{
  SF_DLSYM(&_sf_surface_set_alpha, "sf_surface_set_alpha");
  return (*_sf_surface_set_alpha)(surface, alpha);
}

SfClient* sf_client_create(void)
{
 SF_DLSYM(&_sf_client_create, "sf_client_create");
 return (*_sf_client_create)();

}

EGLDisplay sf_client_get_egl_display(SfClient* client)
{
 SF_DLSYM(&_sf_client_get_egl_display, "sf_client_get_egl_display");
 return (*_sf_client_get_egl_display)(client);
}

EGLConfig sf_client_get_egl_config(SfClient* client)
{
 SF_DLSYM(&_sf_client_get_egl_config, "sf_client_get_egl_config");
 return (*_sf_client_get_egl_config)(client);
}

void sf_client_begin_transaction(SfClient* client)
{
 SF_DLSYM(&_sf_client_begin_transaction, "sf_client_begin_transaction");
 return (*_sf_client_begin_transaction)(client);
}

void sf_client_end_transaction(SfClient* client)
{
 SF_DLSYM(&_sf_client_end_transaction, "sf_client_end_transaction");
 return (*_sf_client_end_transaction)(client);
}

}
