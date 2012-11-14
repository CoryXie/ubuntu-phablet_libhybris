#include <ubuntu/ui/ubuntu_ui_session_service.h>

#include <stdio.h>
#include <string.h>

void init_with_total_session_count(unsigned int session_count, void*)
{
    printf("%s: %d\n", __PRETTY_FUNCTION__, session_count);
}

void for_each_session(ubuntu_ui_session_properties props, ubuntu_ui_session_preview_provider, void*)
{
    printf("%s:\n\t Id: %s \n\t Name: %s \n\t Desktop file hint: %s \n", 
           __PRETTY_FUNCTION__, 
           ubuntu_ui_session_properties_get_application_instance_id(props),
           ubuntu_ui_session_properties_get_application_name(props),
           ubuntu_ui_session_properties_get_desktop_file_hint(props));
}

int main(int argc, char** argv)
{
    ubuntu_ui_session_enumerator enumerator;
    memset(&enumerator, 0, sizeof(enumerator));
    
    enumerator.init_with_total_session_count = init_with_total_session_count;
    enumerator.for_each_session = for_each_session;

    ubuntu_ui_session_enumerate_running_sessions(&enumerator);

    return 0;
}

