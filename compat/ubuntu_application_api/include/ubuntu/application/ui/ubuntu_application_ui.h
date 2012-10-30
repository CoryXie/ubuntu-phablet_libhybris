#ifndef UBUNTU_APPLICATION_UI_H_
#define UBUNTU_APPLICATION_UI_H_

#include "ubuntu/application/ui/input/event.h"

#include <EGL/egl.h>

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

    typedef void* ubuntu_application_ui_physical_display_info;
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
    ubuntu_application_ui_create_display_info(
        ubuntu_application_ui_physical_display_info* info,
        size_t index);

    void
    ubuntu_application_ui_destroy_display_info(
        ubuntu_application_ui_physical_display_info info);

    int32_t
    ubuntu_application_ui_query_horizontal_resolution(
        ubuntu_application_ui_physical_display_info info);

    int32_t
    ubuntu_application_ui_query_vertical_resolution(
        ubuntu_application_ui_physical_display_info info);

    void 
    ubuntu_application_ui_create_surface(
        ubuntu_application_ui_surface* out_surface,
        const char* title,
        int width,
        int height,
        SurfaceRole role,
        input_event_cb cb,
        void* ctx);

    void 
    ubuntu_application_ui_destroy_surface(
        ubuntu_application_ui_surface surface);

    EGLNativeWindowType
    ubuntu_application_ui_surface_to_native_window_type(
        ubuntu_application_ui_surface surface);

    void ubuntu_application_ui_move_surface_to(
        ubuntu_application_ui_surface surface,
        int x,
        int y);

    void ubuntu_application_ui_resize_surface_to(
        ubuntu_application_ui_surface surface,
        int w,
        int h);

#ifdef __cplusplus
}
#endif

#endif // UBUNTU_APPLICATION_UI_H_
