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

// C-API implementation
namespace
{
ubuntu::application::ui::Session::Ptr session;

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

template<typename T>
struct Holder
{
    Holder(const T&value = T()) : value(value)
    {
    }

    T value;
};

template<typename T>
Holder<T>* make_holder(const T& value)
{
    return new Holder<T>(value);
}

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
        "TestSession" // TODO { name }
    };

    session = ubuntu::ui::SessionService::instance()->start_a_new_session(creds);
}

void
ubuntu_application_ui_create_display_info(
    ubuntu_application_ui_physical_display_info* info,
    size_t index)
{
    *info = make_holder(
        session->physical_display_info(
            static_cast<ubuntu::application::ui::PhysicalDisplayIdentifier>(index)));    
}

void
ubuntu_application_ui_destroy_display_info(
    ubuntu_application_ui_physical_display_info info)
{
    auto s = static_cast<Holder<ubuntu::application::ui::PhysicalDisplayInfo::Ptr>*>(info);
    delete s;
}

int32_t
ubuntu_application_ui_query_horizontal_resolution(
    ubuntu_application_ui_physical_display_info info)
{
    auto s = static_cast<Holder<ubuntu::application::ui::PhysicalDisplayInfo::Ptr>*>(info);
    return s->value->horizontal_resolution();
}

int32_t
ubuntu_application_ui_query_vertical_resolution(
    ubuntu_application_ui_physical_display_info info,
    size_t index)
{
    auto s = static_cast<Holder<ubuntu::application::ui::PhysicalDisplayInfo::Ptr>*>(info);
    return s->value->vertical_resolution();
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
    
    *out_surface = make_holder(surface);
}

void 
ubuntu_application_ui_destroy_surface(
    ubuntu_application_ui_surface surface)
{
    auto s = static_cast<Holder<ubuntu::application::ui::Surface::Ptr>*>(surface);

    delete s;
}

EGLNativeWindowType
ubuntu_application_ui_surface_to_native_window_type(
    ubuntu_application_ui_surface surface)
{
    auto s = static_cast<Holder<ubuntu::application::ui::Surface::Ptr>*>(surface);
    return s->value->to_native_window_type();
}

void ubuntu_application_ui_move_surface_to(
    ubuntu_application_ui_surface surface,
    int x,
    int y)
{
    auto s = static_cast<Holder<ubuntu::application::ui::Surface::Ptr>*>(surface);
    s->value->move_to(x, y);
}

void ubuntu_application_ui_resize_surface_to(
    ubuntu_application_ui_surface surface,
    int w,
    int h)
{
    auto s = static_cast<Holder<ubuntu::application::ui::Surface::Ptr>*>(surface);
    s->value->resize(w, h);
}
