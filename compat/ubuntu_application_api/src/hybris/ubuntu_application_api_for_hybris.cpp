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
#include "event_loop.h"
#include "input_consumer_thread.h"

#include <ubuntu/application/ui/init.h>
#include <ubuntu/application/ui/session.h>
#include <ubuntu/application/ui/session_credentials.h>
#include <ubuntu/application/ui/setup.h>
#include <ubuntu/application/ui/surface.h>
#include <ubuntu/application/ui/surface_factory.h>
#include <ubuntu/application/ui/surface_properties.h>

#include <ubuntu/ui/session_enumerator.h>
#include <ubuntu/ui/session_service.h>
#include <ubuntu/ui/well_known_applications.h>

#include <binder/IMemory.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <surfaceflinger/SurfaceComposerClient.h>
#include <ui/InputTransport.h>
#include <ui/PixelFormat.h>
#include <ui/Region.h>
#include <ui/Rect.h>
#include <utils/Looper.h>

namespace android
{

struct Setup : public ubuntu::application::ui::Setup
{
    Setup() : stage(ubuntu::application::ui::main_stage),
        form_factor(ubuntu::application::ui::desktop_form_factor),
        desktop_file("/usr/share/applications/shotwell.desktop")
    {
    }

    static android::KeyedVector<android::String8, ubuntu::application::ui::StageHint> init_string_to_stage_hint_lut()
    {
        android::KeyedVector<android::String8, ubuntu::application::ui::StageHint> lut;
        lut.add(android::String8("main_stage"), ubuntu::application::ui::main_stage);
        lut.add(android::String8("side_stage"), ubuntu::application::ui::side_stage);
        lut.add(android::String8("share_stage"), ubuntu::application::ui::share_stage);

        return lut;
    }

    static ubuntu::application::ui::StageHint string_to_stage_hint(const android::String8& s)
    {
        static android::KeyedVector<android::String8, ubuntu::application::ui::StageHint> lut = init_string_to_stage_hint_lut();

        return lut.valueFor(s);
    }

    static android::KeyedVector<android::String8, ubuntu::application::ui::FormFactorHint> init_string_to_form_factor_hint_lut()
    {
        android::KeyedVector<android::String8, ubuntu::application::ui::FormFactorHint> lut;
        lut.add(android::String8("desktop"), ubuntu::application::ui::desktop_form_factor);
        lut.add(android::String8("phone"), ubuntu::application::ui::phone_form_factor);
        lut.add(android::String8("tablet"), ubuntu::application::ui::tablet_form_factor);

        return lut;
    }

    static ubuntu::application::ui::FormFactorHint string_to_form_factor_hint(const android::String8& s)
    {
        static android::KeyedVector<android::String8, ubuntu::application::ui::FormFactorHint> lut = init_string_to_form_factor_hint_lut();

        return lut.valueFor(s);
    }

    ubuntu::application::ui::StageHint stage_hint()
    {
        return ubuntu::application::ui::main_stage;
    }

    ubuntu::application::ui::FormFactorHintFlags form_factor_hint()
    {
        return ubuntu::application::ui::desktop_form_factor;
    }

    const char* desktop_file_hint()
    {
        return desktop_file.string();
    }

    ubuntu::application::ui::StageHint stage;
    ubuntu::application::ui::FormFactorHintFlags form_factor;
    android::String8 desktop_file;
};

static Setup::Ptr global_setup(new Setup());

struct PhysicalDisplayInfo : public ubuntu::application::ui::PhysicalDisplayInfo
{
    explicit PhysicalDisplayInfo(size_t display_id) : display_id(display_id)
    {
    }

    int dpi()
    {
        return 96;
    }

    int horizontal_resolution()
    {
        return SurfaceComposerClient::getDisplayWidth(display_id);
    }

    int vertical_resolution()
    {
        return SurfaceComposerClient::getDisplayHeight(display_id);
    }

    size_t display_id;
};

struct UbuntuSurface : public ubuntu::application::ui::Surface
{
    sp<SurfaceComposerClient> client;
    sp<SurfaceControl> surface_control;
    sp<android::Surface> surface;
    sp<InputChannel> input_channel;
    InputConsumer input_consumer;
    sp<Looper> looper;
    PreallocatedInputEventFactory event_factory;
    IApplicationManagerSession::SurfaceProperties properties;

    bool is_visible_flag;

    static int looper_callback(int receiveFd, int events, void* ctxt)
    {
        bool result = true;
        UbuntuSurface* s = static_cast<UbuntuSurface*>(ctxt);
        InputEvent* ev;

        s->input_consumer.receiveDispatchSignal();

        switch(s->input_consumer.consume(&s->event_factory, &ev))
        {
        case OK:
            result = true;
            //printf("We have a client side event for process %d. \n", getpid());
            s->translate_and_dispatch_event(ev);
            s->input_consumer.sendFinishedSignal(result);
            break;
        case INVALID_OPERATION:
            result = true;
            break;
        case NO_MEMORY:
            result = true;
            break;
        }

        return result ? 1 : 0;
    }

    UbuntuSurface(const sp<SurfaceComposerClient>& client,
                  const sp<InputChannel>& input_channel,
                  const sp<Looper>& looper,
                  const ubuntu::application::ui::SurfaceProperties& props,
                  const ubuntu::application::ui::input::Listener::Ptr& listener)
        : ubuntu::application::ui::Surface(listener),
          client(client),
          input_channel(input_channel),
          input_consumer(input_channel),
          looper(looper),
        properties( {0, 0, 0, props.width-1, props.height-1}),
    is_visible_flag(false)
    {
        assert(client != NULL);

        surface_control = client->createSurface(
                              String8(props.title),
                              ubuntu::application::ui::primary_physical_display,
                              props.width,
                              props.height,
                              PIXEL_FORMAT_RGBA_8888,
                              0x300);

        assert(surface_control != NULL);

        surface = surface_control->getSurface();

        assert(surface != NULL);

        // Setup input channel
        input_consumer.initialize();
        looper->addFd(input_channel->getReceivePipeFd(),
                      0,
                      ALOOPER_EVENT_INPUT,
                      looper_callback,
                      this);
    }

    ~UbuntuSurface()
    {
        looper->removeFd(input_channel->getReceivePipeFd());
    }

    void translate_and_dispatch_event(const android::InputEvent* ev)
    {
        Event e;
        switch(ev->getType())
        {
        case AINPUT_EVENT_TYPE_KEY:
        {
            const android::KeyEvent* kev = static_cast<const android::KeyEvent*>(ev);
            e.type = KEY_EVENT_TYPE;
            e.device_id = ev->getDeviceId();
            e.source_id = ev->getSource();
            e.action = kev->getAction();
            e.flags = kev->getFlags();
            e.meta_state = kev->getMetaState();
            e.details.key.key_code = kev->getKeyCode();
            e.details.key.scan_code = kev->getScanCode();
            e.details.key.repeat_count = kev->getRepeatCount();
            e.details.key.down_time = kev->getDownTime();
            e.details.key.event_time = kev->getEventTime();
            e.details.key.is_system_key = kev->isSystemKey();
            break;
        }
        case AINPUT_EVENT_TYPE_MOTION:
            const android::MotionEvent* mev = static_cast<const android::MotionEvent*>(ev);
            e.type = MOTION_EVENT_TYPE;
            e.device_id = ev->getDeviceId();
            e.source_id = ev->getSource();
            e.action = mev->getAction();
            e.flags = mev->getFlags();
            e.meta_state = mev->getMetaState();
            e.details.motion.edge_flags = mev->getEdgeFlags();
            e.details.motion.button_state = mev->getButtonState();
            e.details.motion.x_offset = mev->getXOffset();
            e.details.motion.y_offset = mev->getYOffset();
            e.details.motion.x_precision = mev->getXPrecision();
            e.details.motion.y_precision = mev->getYPrecision();
            e.details.motion.down_time = mev->getDownTime();
            e.details.motion.event_time = mev->getEventTime();
            e.details.motion.pointer_count = mev->getPointerCount();
            for(unsigned int i = 0; i < mev->getPointerCount(); i++)
            {
                e.details.motion.pointer_coordinates[i].id = mev->getPointerId(i);
                e.details.motion.pointer_coordinates[i].x = mev->getX(i);
                e.details.motion.pointer_coordinates[i].raw_x = mev->getRawX(i);
                e.details.motion.pointer_coordinates[i].y = mev->getY(i);
                e.details.motion.pointer_coordinates[i].raw_y = mev->getRawY(i);
                e.details.motion.pointer_coordinates[i].touch_major = mev->getTouchMajor(i);
                e.details.motion.pointer_coordinates[i].touch_minor = mev->getTouchMinor(i);
                e.details.motion.pointer_coordinates[i].size = mev->getSize(i);
                e.details.motion.pointer_coordinates[i].pressure = mev->getPressure(i);
                e.details.motion.pointer_coordinates[i].orientation = mev->getOrientation(i);
            }
            break;
        }

        registered_input_listener()->on_new_event(e);
    }

    void set_layer(int layer)
    {
        client->openGlobalTransaction();
        surface_control->setLayer(layer);
        client->closeGlobalTransaction();
        properties.layer = layer;
    }

    void set_visible(bool visible)
    {
        LOGI("%s: %s", __PRETTY_FUNCTION__, visible ? "true" : "false");
        if (visible)
        {
            client->openGlobalTransaction();
            LOGI("surface_control->show(INT_MAX): %d", surface_control->show());
            client->closeGlobalTransaction();
        }
        else
        {
            client->openGlobalTransaction();
            LOGI("surface_control->hide(): %d", surface_control->hide());
            client->closeGlobalTransaction();
        }
    }

    void set_alpha(float alpha)
    {
        client->openGlobalTransaction();
        surface_control->setAlpha(alpha);
        client->closeGlobalTransaction();
    }

    void move_to(int x, int y)
    {
        client->openGlobalTransaction();
        surface_control->setPosition(x, y);
        client->closeGlobalTransaction();
        properties.left = x;
        properties.top = y;
        properties.right += x;
        properties.bottom += y;
    }

    void resize(int w, int h)
    {
        client->openGlobalTransaction();
        surface_control->setSize(w, h);
        client->closeGlobalTransaction();
        properties.right = properties.left + w;
        properties.bottom = properties.top + h;
    }

    EGLNativeWindowType to_native_window_type()
    {
        return surface.get();
    }
};

struct Session : public ubuntu::application::ui::Session
{
    struct ApplicationManagerSession : public BnApplicationManagerSession
    {
        ApplicationManagerSession(Session* parent) : parent(parent)
        {
        }

        // From IApplicationManagerSession
        void raise_application_surfaces_to_layer(int layer)
        {
            LOGI("%s: %d \n", __PRETTY_FUNCTION__, layer);
            parent->raise_application_surfaces_to_layer(layer);
        }

        void raise_surface_to_layer(int32_t token, int layer)
        {
            LOGI("Enter %s (%d): %d, %d", __PRETTY_FUNCTION__, getpid(), token, layer);

            auto surface = parent->surfaces.valueFor(token);
            if (surface != NULL)
            {
                LOGI("\tFound surface for token: %d", token);
                surface->set_layer(layer);
            } else
            {
                LOGI("\tFound NO surface for token: %d", token);
            }
            
            LOGI("Leave %s (%d): %d, %d", __PRETTY_FUNCTION__, getpid(), token, layer);
        }

        SurfaceProperties query_surface_properties_for_token(int32_t token)
        {
            LOGI("%s: %d \n", __PRETTY_FUNCTION__, token);
            return parent->surfaces.valueFor(token)->properties;
        }

        Session* parent;
    };

    sp<ApplicationManagerSession> app_manager_session;
    sp<SurfaceComposerClient> client;
    sp<Looper> looper;
    sp<ubuntu::application::EventLoop> event_loop;
    Mutex surfaces_guard;
    KeyedVector< int32_t, android::sp<UbuntuSurface> > surfaces;

    Session(const ubuntu::application::ui::SessionCredentials& creds)
        : app_manager_session(new ApplicationManagerSession(this)),
          client(new android::SurfaceComposerClient()),
          looper(new Looper(true)),
          event_loop(new ubuntu::application::EventLoop(looper))
    {
        assert(client);
        //============= This has to die =================
        sp<InputChannel> server_channel, client_channel;
        InputChannel::openInputChannelPair(
            String8("UbuntuApplicationUiSession"),
            server_channel,
            client_channel);

        //printf("Created input channels: \n");
        //printf("\t %d, %d, %d \n",
        //server_channel->getAshmemFd(),
        //server_channel->getSendPipeFd(),
        //server_channel->getReceivePipeFd());
        //============= This has to die =================
        sp<IServiceManager> service_manager = defaultServiceManager();
        sp<IBinder> service = service_manager->getService(
                                  String16(IApplicationManager::exported_service_name()));
        BpApplicationManager app_manager(service);

        app_manager.start_a_new_session(
            creds.session_type(),
            String8(creds.application_name()),
            String8(ubuntu::application::ui::Setup::instance()->desktop_file_hint()),
            app_manager_session,
            server_channel->getAshmemFd(),
            server_channel->getSendPipeFd(),
            server_channel->getReceivePipeFd());

        android::ProcessState::self()->startThreadPool();
        event_loop->run();
    }

    ubuntu::application::ui::PhysicalDisplayInfo::Ptr physical_display_info(
        ubuntu::application::ui::PhysicalDisplayIdentifier id)
    {
        ubuntu::application::ui::PhysicalDisplayInfo::Ptr display(
            new PhysicalDisplayInfo(static_cast<size_t>(id)));

        return display;
    }

    ubuntu::application::ui::Surface::Ptr create_surface(
        const ubuntu::application::ui::SurfaceProperties& props,
        const ubuntu::application::ui::input::Listener::Ptr& listener)
    {
        sp<IServiceManager> service_manager = defaultServiceManager();
        sp<IBinder> service = service_manager->getService(
                                  String16(IApplicationManager::exported_service_name()));
        BpApplicationManager app_manager(service);

        sp<InputChannel> server_channel, client_channel;

        InputChannel::openInputChannelPair(
            String8(props.title),
            server_channel,
            client_channel);

        UbuntuSurface* surface = new UbuntuSurface(
            client,
            client_channel,
            looper,
            props,
            listener);

        int32_t token;

        {
            Mutex::Autolock al(surfaces_guard);
            token = next_surface_token();
            surfaces.add(token, sp<UbuntuSurface>(surface));
        }

        app_manager.register_a_surface(
            String8(props.title),
            app_manager_session,
            props.role,
            token,
            server_channel->getAshmemFd(),
            server_channel->getSendPipeFd(),
            server_channel->getReceivePipeFd());

        return ubuntu::application::ui::Surface::Ptr(surface);
    }

    void destroy_surface(
        const ubuntu::application::ui::Surface::Ptr& surf)
    {
        (void) surf;
    }

    void raise_application_surfaces_to_layer(int layer)
    {
        Mutex::Autolock al(surfaces_guard);
        LOGI("%s: %d\n", __PRETTY_FUNCTION__, layer);
        for(size_t i = 0; i < surfaces.size(); i++)
        {
            surfaces.valueAt(i)->set_layer(layer+i);
            LOGI("\tLayer: %d\n", layer+i);
        }
    }

    int32_t next_surface_token()
    {
        static int32_t t = 0;
        t++;
        return t;
    }
};

struct SessionProperties : public ubuntu::ui::SessionProperties
{
    SessionProperties(int id, const android::String8& desktop_file)
        : id(id),
          desktop_file(desktop_file)
    {
    }

    int application_instance_id() const
    {
        return id;
    }

    const char* value_for_key(const char* key) const
    {
        return "lalelu";
    }

    virtual const char* desktop_file_hint() const
    {
        return desktop_file.string();
    }

    int id;
    android::String8 desktop_file;
};

struct ApplicationManagerObserver : public android::BnApplicationManagerObserver
{
    void on_session_born(int id,
                         const String8& desktop_file)
    {
        if (observer == NULL)
            return;

        observer->on_session_born(ubuntu::ui::SessionProperties::Ptr(new SessionProperties(id, desktop_file)));
    }

    virtual void on_session_focused(int id,
                                    const String8& desktop_file)
    {
        if (observer == NULL)
            return;

        observer->on_session_focused(ubuntu::ui::SessionProperties::Ptr(new SessionProperties(id, desktop_file)));
    }

    virtual void on_session_died(int id,
                                 const String8& desktop_file)
    {
        if (observer == NULL)
            return;

        observer->on_session_died(ubuntu::ui::SessionProperties::Ptr(new SessionProperties(id, desktop_file)));
    }

    void install_session_lifecycle_observer(const ubuntu::ui::SessionLifeCycleObserver::Ptr& observer)
    {
        this->observer = observer;

        sp<IServiceManager> service_manager = defaultServiceManager();
        sp<IBinder> service = service_manager->getService(
                                  String16(IApplicationManager::exported_service_name()));
        BpApplicationManager app_manager(service);

        app_manager.register_an_observer(android::sp<IApplicationManagerObserver>(this));
    }

    ubuntu::ui::SessionLifeCycleObserver::Ptr observer;
};

struct SessionService : public ubuntu::ui::SessionService
{    
    struct SessionSnapshot : public ubuntu::ui::SessionSnapshot
    {
        const void* snapshot_pixels;
        unsigned int snapshot_width;
        unsigned int snapshot_height;
        unsigned int snapshot_stride;
        
        SessionSnapshot(
            const void* pixels, 
            unsigned int width, 
            unsigned height, 
            unsigned int stride) : snapshot_pixels(pixels),
                                   snapshot_width(width),
                                   snapshot_height(height),
                                   snapshot_stride(stride)
        {
        }

        const void* pixel_data() {
            return snapshot_pixels;
        }
        
        unsigned int width() { return snapshot_width; }
        unsigned int height() { return snapshot_height; }
        unsigned int stride() { return snapshot_stride; }       
    };

    static sp<BpApplicationManager> access_application_manager()
    {
        static sp<BpApplicationManager> remote_instance;

        if (remote_instance == NULL)
        {
            sp<IServiceManager> service_manager = defaultServiceManager();
            sp<IBinder> service = service_manager->getService(
                String16(IApplicationManager::exported_service_name()));
            remote_instance = new BpApplicationManager(service);
        }

        return remote_instance;
    }

    SessionService() : observer(new ApplicationManagerObserver())
    {
        android::ProcessState::self()->startThreadPool();
    }

    const ubuntu::application::ui::Session::Ptr& start_a_new_session(const ubuntu::application::ui::SessionCredentials& cred)
    {
        (void) cred;
        static ubuntu::application::ui::Session::Ptr session(new Session(cred));
        return session;
    }

    void install_session_lifecycle_observer(const ubuntu::ui::SessionLifeCycleObserver::Ptr& lifecycle_observer)
    {
        this->observer->install_session_lifecycle_observer(lifecycle_observer);
    }

    void focus_running_session_with_id(int id)
    {
        access_application_manager()->focus_running_session_with_id(id);
    }

    ubuntu::ui::SessionSnapshot::Ptr snapshot_running_session_with_id(int id)
    {
        static const unsigned int default_width = 720;
        static const unsigned int default_height = 1280;
        int32_t layer_min = id > 0 
                ? access_application_manager()->query_snapshot_layer_for_session_with_id(id) 
                : 0;
        int32_t layer_max = id > 0 
                ? access_application_manager()->query_snapshot_layer_for_session_with_id(id) 
                : id;  
        static android::ScreenshotClient screenshot_client;
        screenshot_client.update(default_width, default_height, layer_min, layer_max);
 
        SessionSnapshot::Ptr ss(
            new SessionSnapshot(
                screenshot_client.getPixels(),
                screenshot_client.getWidth(),
                screenshot_client.getHeight(),
                screenshot_client.getStride()));
       
        return ss;
    }

    void trigger_switch_to_well_known_application(ubuntu::ui::WellKnownApplication app)
    {
        access_application_manager()->switch_to_well_known_application(app);
    }

    android::sp<ApplicationManagerObserver> observer;
};

}

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

// We need to inject some platform specific symbols here.
namespace ubuntu
{
namespace application
{
namespace ui
{
void init(int argc, char** argv)
{
    static struct option long_options[] =
    {
        {"form_factor_hint", required_argument, 0, 'f'},
        {"stage_hint", required_argument, 0, 's'},
        {"desktop_file_hint", required_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    static const int form_factor_hint_index = 0;
    static const int stage_hint_index = 1;
    static const int desktop_file_hint_index = 2;

    android::Setup* setup = new android::Setup();

    while(true)
    {
        int option_index = 0;

        int c = getopt_long(argc,
                            argv,
                            "s:d:f:",
                            long_options,
                            &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            // If this option set a flag, do nothing else now.
            if (long_options[option_index].flag != 0)
                break;
            if (optarg)
            {
                switch(option_index)
                {
                case form_factor_hint_index:
                    setup->form_factor = android::Setup::string_to_form_factor_hint(android::String8(optarg));
                    break;
                case stage_hint_index:
                    setup->stage = android::Setup::string_to_stage_hint(android::String8(optarg));
                    break;
                case desktop_file_hint_index:
                    setup->desktop_file = android::String8(optarg);
                    break;
                }
                printf (" with arg %s", optarg);
            }
            printf ("\n");
            break;
        case 's':
            printf ("option -s with value `%s'\n", optarg);
            break;
        case 'd':
            printf ("option -d with value `%s'\n", optarg);
            break;
        case 'f':
            printf ("option -f with value `%s'\n", optarg);
            break;

        case '?':
            break;
        }
    }

    android::global_setup = setup;
}

const ubuntu::application::ui::Setup::Ptr& ubuntu::application::ui::Setup::instance()
{
    return android::global_setup;
}

}
}
namespace ui
{
const ubuntu::ui::SessionService::Ptr& ubuntu::ui::SessionService::instance()
{
    static ubuntu::ui::SessionService::Ptr instance(new android::SessionService());
    return instance;
}

const char* SessionProperties::key_application_instance_id()
{
    static const char* key = "application_instance_id";
    return key;
}

const char* SessionProperties::key_application_name()
{
    static const char* key = "application_name";
    return key;
}

const char* SessionProperties::key_desktop_file_hint()
{
    static const char* key = "desktop_file_hint";
    return key;
}
}
}
