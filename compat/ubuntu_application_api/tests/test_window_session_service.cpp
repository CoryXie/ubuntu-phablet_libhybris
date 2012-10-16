#include <ubuntu/ui/session_service.h>

#include <ubuntu/application/ui/init.h>
#include <ubuntu/application/ui/input/listener.h>
#include <ubuntu/application/ui/session.h>
#include <ubuntu/application/ui/session_credentials.h>
#include <ubuntu/application/ui/setup.h>
#include <ubuntu/application/ui/surface.h>
#include <ubuntu/application/ui/surface_factory.h>
#include <ubuntu/application/ui/surface_properties.h>

#include <iostream>

namespace
{
struct InputListener : public ubuntu::application::ui::input::Listener
{
    void on_new_event(const Event& /*ev*/)
    {
    }
};
}

int main(int argc, char** argv)
{
    ubuntu::application::ui::init(argc, argv);

    std::cout << "We are playing on the ";
    switch(ubuntu::application::ui::Setup::instance()->stage_hint())
    {
        case ubuntu::application::ui::main_stage:
            std::cout << "main stage ";
            break;
        case ubuntu::application::ui::integration_stage:
            std::cout << " integration stage ";
            break;
        case ubuntu::application::ui::share_stage:
            std::cout << " share stage ";
            break;
        case ubuntu::application::ui::content_picking_stage:
            std::cout << " content picking stage ";
            break;
    }

    std::cout << " while running in ";
    switch(ubuntu::application::ui::Setup::instance()->form_factor_hint())
    {
        case ubuntu::application::ui::desktop_form_factor:
            std::cout << "full blown desktop-mode." << std::endl;
            break;
        case ubuntu::application::ui::phone_form_factor:
            std::cout << "phone mode." << std::endl;
            break;
        case ubuntu::application::ui::tablet_form_factor:
            std::cout << "tablet mode." << std::endl;
            break;
    }

    std::cout << "Initiating a session with the window system." << std::endl;
    ubuntu::application::ui::SessionCredentials credentials = {__PRETTY_FUNCTION__};
    ubuntu::application::ui::Session::Ptr session = 
            ubuntu::ui::SessionService::instance()->start_a_new_session(credentials);

    std::cout << "Setting up the application's actors now." << std::endl;
    ubuntu::application::ui::SurfaceProperties surface_props = 
    {
        "test",
        400,
        400,
        ubuntu::application::ui::main_actor_role
    };

    ubuntu::application::ui::Surface::Ptr surface = 
            session->create_surface(
            surface_props,
                ubuntu::application::ui::input::Listener::Ptr(new InputListener()));
}
