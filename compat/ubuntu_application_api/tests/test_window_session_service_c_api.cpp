/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#include <ubuntu/application/ui/ubuntu_application_ui.h>

#include <stdio.h>
#include <stddef.h>

void on_new_input_event(void* ctx, const Event* e)
{
    (void) ctx;
    (void) e;
}

int main(int argc, char** argv)
{
    ubuntu_application_ui_init(argc, argv);

    printf("We are playing on the ");
    switch(ubuntu_application_ui_setup_get_stage_hint())
    {
    case MAIN_STAGE_HINT:
        printf("main stage ");
        break;
    case INTEGRATION_STAGE_HINT:
        printf(" integration stage ");
        break;
    case SHARE_STAGE_HINT:
        printf(" share stage ");
        break;
    case CONTENT_PICKING_STAGE_HINT:
        printf(" content picking stage ");
        break;
    }

    printf(" while running in ");
    switch(ubuntu_application_ui_setup_get_form_factor_hint())
    {
    case DESKTOP_FORM_FACTOR_HINT:
        printf("full blown desktop-mode.\n");
        break;
    case PHONE_FORM_FACTOR_HINT:
        printf("phone mode.\n");
        break;
    case TABLET_FORM_FACTOR_HINT:
        printf("tablet mode.\n");
        break;
    }

    printf("Initiating a session with the windowing system...");
    ubuntu_application_ui_start_a_new_session(__PRETTY_FUNCTION__);
    printf("done.\n");

    printf("Setting up the application's actors...");
    ubuntu_application_ui_surface surface;
    ubuntu_application_ui_create_surface(
        &surface,
        "MainActorSurface",
        400,
        400,
        MAIN_ACTOR_ROLE,
        on_new_input_event,
        NULL);
    printf("...done.\n");
}
