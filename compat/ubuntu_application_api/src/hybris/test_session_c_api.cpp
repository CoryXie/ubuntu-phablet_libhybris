#include <ubuntu/ui/ubuntu_ui_session_service.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef union
{
    struct Components
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } components;
    uint32_t value;
} Pixel;

void on_snapshot_completed(const void* pixel_data, unsigned int width, unsigned int height, unsigned int stride, void* context)
{
    static unsigned int counter = 0;
    
    printf("%s: (%p, %d, %d, %d) \n",
           __PRETTY_FUNCTION__,
           pixel_data,
           width,
           height,
           stride);

    char fn[256];
    snprintf(fn, 256, "./snapshot_%d.pgm", counter); counter++;
    FILE* f = fopen(fn, "w+");

    if (!f)
    {
        printf("Problem opening file: %s \n", fn);
        return;
    }
        
    const unsigned int* p = static_cast<const unsigned int*>(pixel_data);

    fprintf(f, "P2\n%d %d\n%d\n\n", width, height, 255);
    for(unsigned int i = 0; i < height; i++)
    {
        for(unsigned int j = 0; j < width; j++)
        {
            Pixel pixel; pixel.value = *p; ++p;
            fprintf(
                f, "%d ", 
                (unsigned short) (0.21*pixel.components.r + 0.71*pixel.components.g + 0.07*pixel.components.b));
        }
    }
}

void on_session_born(ubuntu_ui_session_properties props, void*)
{
    printf("%s:\n\t Id: %d \n\t Desktop file hint: %s \n",
           __PRETTY_FUNCTION__,
           ubuntu_ui_session_properties_get_application_instance_id(props),
           ubuntu_ui_session_properties_get_desktop_file_hint(props));

    ubuntu_ui_session_snapshot_running_session_with_id(
        ubuntu_ui_session_properties_get_application_instance_id(props),
        on_snapshot_completed,
        NULL);
}

void on_session_focused(ubuntu_ui_session_properties props, void*)
{
    printf("%s:\n\t Id: %d \n\t Desktop file hint: %s \n",
           __PRETTY_FUNCTION__,
           ubuntu_ui_session_properties_get_application_instance_id(props),
           ubuntu_ui_session_properties_get_desktop_file_hint(props));
}

void on_session_died(ubuntu_ui_session_properties props, void*)
{
    printf("%s:\n\t Id: %d \n\t Desktop file hint: %s \n",
           __PRETTY_FUNCTION__,
           ubuntu_ui_session_properties_get_application_instance_id(props),
           ubuntu_ui_session_properties_get_desktop_file_hint(props));
}

int main(int argc, char** argv)
{
    ubuntu_ui_session_lifecycle_observer observer;

    memset(&observer, 0, sizeof(observer));
    observer.on_session_born = on_session_born;
    observer.on_session_focused = on_session_focused;
    observer.on_session_died = on_session_died;

    ubuntu_ui_session_install_session_lifecycle_observer(&observer);

    while(true)
    {
    }

    return 0;
}

