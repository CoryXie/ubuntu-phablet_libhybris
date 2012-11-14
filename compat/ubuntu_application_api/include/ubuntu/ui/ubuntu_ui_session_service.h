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
        typedef void (*application_born_cb)(ubuntu_ui_session_properties props, void* context);
        typedef void (*application_died_cb)(ubuntu_ui_session_properties props, void * context);

        application_born_cb on_application_born;
        application_died_cb on_application_died;

        void* context;
    } ubuntu_ui_session_lifecycle_observer;

    typedef struct
    {
        typedef void (*init_with_total_session_count_cb)(unsigned int session_count, void* context);
        typedef void (*for_each_session_cb)(
            ubuntu_ui_session_properties props, 
            ubuntu_ui_session_preview_provider preview_provider, 
            void* context);

        init_with_total_session_count_cb init_with_total_session_count;
        for_each_session_cb for_each_session;
        void* context;
    } ubuntu_ui_session_enumerator;

    const char* ubuntu_ui_session_properties_get_value_for_key(ubuntu_ui_session_properties props, const char* key);
    const char* ubuntu_ui_session_properties_get_application_instance_id(ubuntu_ui_session_properties props);
    const char* ubuntu_ui_session_properties_get_application_name(ubuntu_ui_session_properties props);
    const char* ubuntu_ui_session_properties_get_desktop_file_hint(ubuntu_ui_session_properties props);

    bool ubuntu_ui_session_preview_provider_update_session_preview_texture(ubuntu_ui_session_preview_provider pp, GLuint texture, unsigned int* width, unsigned int* height);

    void ubuntu_ui_session_install_session_lifecycle_observer(ubuntu_ui_session_lifecycle_observer* observer);
    void ubuntu_ui_session_enumerate_running_sessions(ubuntu_ui_session_enumerator* enumerator);

#ifdef __cplusplus
}
#endif

#endif // UBUNTU_UI_SESSION_SERVICE_C_API_H_
