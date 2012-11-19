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
#ifndef DEFAULT_APPLICATION_SESSION_H_
#define DEFAULT_APPLICATION_SESSION_H_

#include "application_manager.h"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>

namespace mir
{

struct ApplicationSession : public android::RefBase
{
    struct Surface : public android::RefBase
    {
        Surface(ApplicationSession* parent,
                const android::sp<android::InputChannel>& input_channel,
                int32_t token) : parent(parent),
            input_channel(input_channel),
            token(token)
        {
        }

        android::IApplicationManagerSession::SurfaceProperties query_properties()
        {
            android::IApplicationManagerSession::SurfaceProperties props =
                parent->remote_session->query_surface_properties_for_token(token);

            return props;
        }

        android::sp<android::InputWindowHandle> make_input_window_handle()
        {
            return android::sp<android::InputWindowHandle>(new InputWindowHandle(parent, android::sp<Surface>(this)));
        }

        ApplicationSession* parent;
        android::sp<android::InputChannel> input_channel;
        int32_t token;
    };

    ApplicationSession(
        android::sp<android::IApplicationManagerSession> remote_session,
        const android::String8& app_name)
        : remote_session(remote_session),
          app_name(app_name)
    {
    }

    struct InputApplicationHandle : public android::InputApplicationHandle
    {
        InputApplicationHandle(ApplicationSession* parent) : parent(parent)
        {
            updateInfo();
        }

        bool updateInfo()
        {
            if (mInfo == NULL)
            {
                mInfo = new android::InputApplicationInfo();
                mInfo->name = parent->app_name;
                mInfo->dispatchingTimeout = 10 * 1000 * 1000 * 1000; // TODO(tvoss): Find out sensible value here
            }

            return true;
        }

        ApplicationSession* parent;
    };

    struct InputWindowHandle : public android::InputWindowHandle
    {
        InputWindowHandle(ApplicationSession* parent, const android::sp<Surface>& surface)
            : android::InputWindowHandle(
                android::sp<InputApplicationHandle>(
                    new InputApplicationHandle(parent))),
            parent(parent),
            surface(surface)
        {
            updateInfo();
        }

        bool updateInfo()
        {
            if (mInfo == NULL)
            {
                android::IApplicationManagerSession::SurfaceProperties props = surface->query_properties();

                SkRegion touchable_region;
                touchable_region.setRect(props.left, props.top, props.right, props.bottom);

                mInfo = new android::InputWindowInfo();
                mInfo->name = parent->app_name;
                mInfo->layoutParamsFlags = android::InputWindowInfo::FLAG_SPLIT_TOUCH | android::InputWindowInfo::FLAG_HARDWARE_ACCELERATED;
                mInfo->layoutParamsType = android::InputWindowInfo::TYPE_BASE_APPLICATION;
                mInfo->touchableRegion = touchable_region;
                mInfo->frameLeft = props.left;
                mInfo->frameTop = props.top;
                mInfo->frameRight = props.right;
                mInfo->frameBottom = props.bottom;
                mInfo->scaleFactor = 1.f;
                mInfo->visible = true;
                mInfo->canReceiveKeys = true;
                mInfo->hasFocus = true;
                mInfo->hasWallpaper = false;
                mInfo->paused = false;
                mInfo->layer = 100;
                mInfo->dispatchingTimeout = 100 * 1000 * 1000 * 1000;
                mInfo->ownerPid = 0;
                mInfo->ownerUid = 0;
                mInfo->inputFeatures = 0;
                mInfo->inputChannel = surface->input_channel;
            }
            return true;
        }

        ApplicationSession* parent;
        android::sp<Surface> surface;
    };

    android::Vector< android::sp<android::InputWindowHandle> > input_window_handles()
    {
        android::Vector< android::sp<android::InputWindowHandle> > v;
        for(size_t i = 0; i < registered_surfaces.size(); i++)
        {
            v.push_back(registered_surfaces.valueAt(i)->make_input_window_handle());
        }

        return v;
    }

    android::sp<android::InputApplicationHandle> input_application_handle()
    {
        return android::sp<android::InputApplicationHandle>(new InputApplicationHandle(this));
    }

    void raise_application_surfaces_to_layer(int layer)
    {
        remote_session->raise_application_surfaces_to_layer(layer);
    }

    void register_surface(const android::sp<Surface>& surface)
    {
        registered_surfaces.add(surface->token, surface);
    }

    android::sp<android::IApplicationManagerSession> remote_session;
    android::String8 app_name;
    android::KeyedVector<int32_t, android::sp<Surface>> registered_surfaces;
};

}
#endif // DEFAULT_APPLICATION_SESSION_H_
