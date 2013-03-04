/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/misc.h>

#include <gui/SurfaceComposerClient.h>
#include <ui/Region.h>
#include <ui/Rect.h>

using namespace android;

static sp<SurfaceComposerClient> android_client;
static sp<Surface> android_surface;
static sp<SurfaceControl> android_surface_control;

void hw_setup(sp<Surface> surf, int w, int h);
void hw_render();
void hw_step();
