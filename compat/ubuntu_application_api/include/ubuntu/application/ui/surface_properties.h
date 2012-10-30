#ifndef UBUNTU_APPLICATION_UI_SURFACE_PROPERTIES_H_
#define UBUNTU_APPLICATION_UI_SURFACE_PROPERTIES_H_

#include "ubuntu/application/ui/surface_role.h"

namespace ubuntu
{
namespace application
{
namespace ui
{
struct SurfaceProperties
{
    enum { 
        max_surface_title_length = 512
    };
        
    const char title[max_surface_title_length];
    int width;
    int height;
    SurfaceRole role;
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_SURFACE_PROPERTIES_H_
