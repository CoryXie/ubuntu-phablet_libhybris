#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/misc.h>

#include <surfaceflinger/SurfaceComposerClient.h>
#include <ui/Region.h>
#include <ui/Rect.h>

using namespace android;

static sp<SurfaceComposerClient> android_client;
static sp<Surface> android_surface;
static sp<SurfaceControl> android_surface_control;

void hw_setup(sp<Surface> surf, int w, int h);
void hw_render();
void hw_step();
