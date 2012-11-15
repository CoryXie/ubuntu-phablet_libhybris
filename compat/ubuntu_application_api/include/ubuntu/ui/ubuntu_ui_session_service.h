#ifndef UBUNTU_UI_SESSION_SERVICE_C_API_H_
#define UBUNTU_UI_SESSION_SERVICE_C_API_H_

#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef const void* ubuntu_ui_session_properties;
    typedef const void* ubuntu_ui_session_preview_provider;
    
    typedef struct
    {
        typedef void (*session_born_cb)(ubuntu_ui_session_properties props, void* context);
        typedef void (*session_focused_cb)(ubuntu_ui_session_properties props, void* context);
        typedef void (*session_died_cb)(ubuntu_ui_session_properties props, void * context);

        session_born_cb on_session_born;
        session_focused_cb on_session_focused;
        session_died_cb on_session_died;

        void* context;
    } ubuntu_ui_session_lifecycle_observer;

    const char* ubuntu_ui_session_properties_get_value_for_key(ubuntu_ui_session_properties props, const char* key);
    int ubuntu_ui_session_properties_get_application_instance_id(ubuntu_ui_session_properties props);
    const char* ubuntu_ui_session_properties_get_desktop_file_hint(ubuntu_ui_session_properties props);

    void ubuntu_ui_session_install_session_lifecycle_observer(ubuntu_ui_session_lifecycle_observer* observer);
    
    bool ubuntu_ui_session_preview_provider_update_session_preview_texture_with_id(
        ubuntu_ui_session_preview_provider pp, 
        int id,
        GLuint texture, 
        unsigned int* width, 
        unsigned int* height);

    void ubuntu_ui_session_focus_running_session_with_id(int id);
    
#ifdef __cplusplus
}
#endif

#endif // UBUNTU_UI_SESSION_SERVICE_C_API_H_
