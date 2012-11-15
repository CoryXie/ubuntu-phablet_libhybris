#include <ubuntu/ui/ubuntu_ui_session_service.h>

#include <stdio.h>
#include <string.h>

void on_session_born(ubuntu_ui_session_properties props, void*)
{
    printf("%s:\n\t Id: %d \n\t Desktop file hint: %s \n", 
           __PRETTY_FUNCTION__, 
           ubuntu_ui_session_properties_get_application_instance_id(props),
           ubuntu_ui_session_properties_get_desktop_file_hint(props));
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

