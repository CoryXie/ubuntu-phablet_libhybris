#include <ubuntu/application/ui/init.h>
#include <ubuntu/application/ui/session.h>
#include <ubuntu/application/ui/session_credentials.h>
#include <ubuntu/application/ui/setup.h>
#include <ubuntu/application/ui/surface.h>
#include <ubuntu/application/ui/surface_factory.h>
#include <ubuntu/application/ui/surface_properties.h>

#include <ubuntu/ui/session_service.h>

// C apis
#include <ubuntu/application/ui/ubuntu_application_ui.h>

#include <set>

// C-API implementation
namespace
{
ubuntu::application::ui::Session::Ptr session;
std::set<ubuntu::application::ui::Surface::Ptr> surfaces;

struct CallbackEventListener : public ubuntu::application::ui::input::Listener
{
    CallbackEventListener(input_event_cb cb, void* context) : cb(cb),
                                                              context(context)
    {
    }

    void on_new_event(const ::Event& e)
    {
        if (cb)
            cb(context, &e);
    }

    input_event_cb cb;
    void* context;
};

}

void 
ubuntu_application_ui_init(int argc, char**argv)
{
    ubuntu::application::ui::init(argc, argv);
}

::StageHint 
ubuntu_application_ui_setup_get_stage_hint()
{
    return static_cast<StageHint>(
        ubuntu::application::ui::Setup::instance()->stage_hint());
}

::FormFactorHint 
ubuntu_application_ui_setup_get_form_factor_hint()
{
    return static_cast<FormFactorHint>(
        ubuntu::application::ui::Setup::instance()->form_factor_hint());
}

void
ubuntu_application_ui_start_a_new_session(const char* name)
{
    if (session != NULL)
        return;

    ubuntu::application::ui::SessionCredentials creds = 
    {
        name
    };

    session = ubuntu::ui::SessionService::instance()->start_a_new_session(creds);
}

void 
ubuntu_application_ui_create_surface(
        ubuntu_application_ui_surface* out_surface,
        const char* title,
        int width,
        int height,
        SurfaceRole role,
        input_event_cb cb,
        void* ctx)
{
    if (session == NULL)
    {
        // TODO: Report the error here.
        return;
    }
    ubuntu::application::ui::SurfaceProperties props = 
    {
        "test",
        width,
        height,
        static_cast<ubuntu::application::ui::SurfaceRole>(role)
    };

    ubuntu::application::ui::Surface::Ptr surface = 
            session->create_surface(
                props,
                ubuntu::application::ui::input::Listener::Ptr(
                    new CallbackEventListener(cb, ctx))); 
    
    surfaces.insert(surface);

    *out_surface = surface.get();
}
