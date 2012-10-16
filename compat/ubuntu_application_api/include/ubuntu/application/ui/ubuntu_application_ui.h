#ifndef UBUNTU_APPLICATION_UI_H_
#define UBUNTU_APPLICATION_UI_H_

#include "ubuntu/application/ui/input/event.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
    {
        MAIN_STAGE_HINT,
        INTEGRATION_STAGE_HINT,
        SHARE_STAGE_HINT,
        CONTENT_PICKING_STAGE_HINT,
        SIDE_STAGE_HINT,
        CONFIGURATION_STAGE_HINT
    } StageHint;

    typedef enum
    {
        DESKTOP_FORM_FACTOR_HINT,
        PHONE_FORM_FACTOR_HINT,
        TABLET_FORM_FACTOR_HINT
    } FormFactorHint;

    typedef enum
    {
        MAIN_ACTOR_ROLE,
        TOOL_SUPPORT_ACTOR_ROLE,
        DIALOG_SUPPORT_ACTOR_ROLE
    } SurfaceRole;
    
    typedef void (*input_event_cb)(void* ctx, const Event* ev);

    typedef void* ubuntu_application_ui_session;
    typedef void* ubuntu_application_ui_surface;
    
    void 
    ubuntu_application_ui_init(
        int argc, 
        char**argv);
    
    StageHint 
    ubuntu_application_ui_setup_get_stage_hint();

    FormFactorHint 
    ubuntu_application_ui_setup_get_form_factor_hint();

    void
    ubuntu_application_ui_start_a_new_session(const char* app_name);

    void 
    ubuntu_application_ui_create_surface(
        ubuntu_application_ui_surface* out_surface,
        const char* title,
        int width,
        int height,
        SurfaceRole role,
        input_event_cb cb,
        void* ctx);

#ifdef __cplusplus
}
#endif

#endif // UBUNTU_APPLICATION_UI_H_
