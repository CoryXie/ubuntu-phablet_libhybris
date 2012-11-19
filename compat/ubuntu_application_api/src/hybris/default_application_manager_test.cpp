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
#include "application_manager.h"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace
{
struct ApplicationManagerSession : public android::BnApplicationManagerSession
{
    ApplicationManagerSession()
    {
    }

    void raise_application_surfaces_to_layer(int layer)
    {
        printf("%s \n", __PRETTY_FUNCTION__);
        printf("%d \n", layer);
    }

    SurfaceProperties query_surface_properties_for_token(int32_t token)
    {
        SurfaceProperties props = { 0, 0, 960, 1280 };
        return props;
    }
};
}

int main(int argc, char** argv)
{
    int test_fd = open("test.file", O_CREAT);

    android::sp<ApplicationManagerSession> session(new ApplicationManagerSession());
    android::sp<android::IServiceManager> service_manager = android::defaultServiceManager();
    android::sp<android::IBinder> service = service_manager->getService(
            android::String16(android::IApplicationManager::exported_service_name()));
    android::BpApplicationManager app_manager(service);

    app_manager.start_a_new_session(
        android::String8("default_application_manager_test"),
        session,
        test_fd,
        test_fd,
        test_fd);

    android::ProcessState::self()->startThreadPool();

    for(;;) {}
}
