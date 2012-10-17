#include "application_manager.h"

#include <ubuntu/application/ui/init.h>
#include <ubuntu/application/ui/session.h>
#include <ubuntu/application/ui/session_credentials.h>
#include <ubuntu/application/ui/setup.h>
#include <ubuntu/application/ui/surface.h>
#include <ubuntu/application/ui/surface_factory.h>
#include <ubuntu/application/ui/surface_properties.h>

#include <ubuntu/ui/session_service.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <surfaceflinger/SurfaceComposerClient.h>
#include <surfaceflinger/SurfaceComposerClient.h>
#include <ui/InputTransport.h>
#include <ui/PixelFormat.h>
#include <ui/Region.h>
#include <ui/Rect.h>
#include <utils/Looper.h>

namespace android
{

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
    
    bool is_visible_flag;

    UbuntuSurface(const sp<SurfaceComposerClient>& client,
                  const ubuntu::application::ui::SurfaceProperties& props,
                  const ubuntu::application::ui::input::Listener::Ptr& listener) 
            : ubuntu::application::ui::Surface(listener),
              client(client),
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
    }

    void set_layer(int layer)
    {
        client->openGlobalTransaction();
        surface_control->setLayer(layer);
        client->closeGlobalTransaction();
    }

    bool is_visible() const
    {
        return is_visible_flag;
    }

    void set_visible(bool visible)
    {
        if (is_visible_flag == visible)
            return;

        is_visible_flag = visible;
        if (is_visible_flag)
        {
            client->openGlobalTransaction();
            surface_control->show();
            client->closeGlobalTransaction();
        } else
        {
            client->openGlobalTransaction();
            surface_control->hide();
            client->closeGlobalTransaction();
        }
    }

    void set_alpha(float alpha)
    {
        client->openGlobalTransaction();
        surface_control->setAlpha(alpha);
        client->closeGlobalTransaction();
    }

    float alpha() const
    {
        return 1.f;
    }

    void move_to(int x, int y)
    {
        client->openGlobalTransaction();
        surface_control->setPosition(x, y);
        client->closeGlobalTransaction();
    }

    void move_by(int dx, int dy)
    {
        //TODO: implement
        (void) dx;
        (void) dy;
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
            printf("%s: %d \n", __PRETTY_FUNCTION__, layer);

            parent->raise_application_surfaces_to_layer(layer);
        }

        Session* parent;
    };

    struct InputConsumerThread : public android::Thread
    {
        InputConsumerThread(android::InputConsumer& input_consumer) 
                : input_consumer(input_consumer),
                  looper(android::Looper::prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS))
        {
            looper->addFd(input_consumer.getChannel()->getReceivePipeFd(),
                          input_consumer.getChannel()->getReceivePipeFd(),
                          ALOOPER_EVENT_INPUT,
                          NULL,
                          NULL);
                         
        }

        bool threadLoop()
        {
            while (true)
            {
                looper->pollOnce(5 * 1000);
                // printf("%s \n", __PRETTY_FUNCTION__);
                InputEvent* event = NULL;
                bool result = true;
                switch(input_consumer.consume(&event_factory, &event))
                {
                    case OK:
                        //TODO:Dispatch to input listener
                        result = true;
                        printf("Yeah, we have an event client-side.\n");
                        input_consumer.sendFinishedSignal(result);
                        break;
                    case INVALID_OPERATION:
                        result = true;
                        break;
                    case NO_MEMORY:
                        result = true;
                        break;
                }                               
            }
            return true;
        }
        
        android::InputConsumer input_consumer;
        android::sp<android::Looper> looper;
        android::PreallocatedInputEventFactory event_factory;
    };

    sp<ApplicationManagerSession> app_manager_session;
    sp<SurfaceComposerClient> client;
    sp<InputChannel> client_channel;
    sp<InputChannel> server_channel;
    InputConsumer input_consumer;
    android::sp<InputConsumerThread> input_consumer_thread;
    Mutex surfaces_guard;
    Vector< android::sp<UbuntuSurface> > surfaces;
    
    Session(const ubuntu::application::ui::SessionCredentials& creds) 
            : app_manager_session(new ApplicationManagerSession(this)),
              client(new android::SurfaceComposerClient()),
              input_consumer(sp<InputChannel>())
    {
        assert(client);

        InputChannel::openInputChannelPair(
            String8("UbuntuApplicationUiSession"),
            server_channel,
            client_channel);

        printf("Created input channels: \n");
        printf("\t %d, %d, %d \n", 
               server_channel->getAshmemFd(),
               server_channel->getSendPipeFd(),
               server_channel->getReceivePipeFd());
        input_consumer = InputConsumer(client_channel);
        input_consumer.initialize();

        input_consumer_thread = new InputConsumerThread(input_consumer);

        sp<IServiceManager> service_manager = defaultServiceManager();
        sp<IBinder> service = service_manager->getService(
            String16(IApplicationManager::exported_service_name()));
        BpApplicationManager app_manager(service);
        
        app_manager.start_a_new_session(
            String8(creds.application_name),
            app_manager_session,
            server_channel->getAshmemFd(),
            server_channel->getSendPipeFd(),
            server_channel->getReceivePipeFd());

        android::ProcessState::self()->startThreadPool();
        input_consumer_thread->run();
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
        UbuntuSurface* surface = new UbuntuSurface(client, props, listener);
        Mutex::Autolock al(surfaces_guard);
        surfaces.push_back(sp<UbuntuSurface>(surface));
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
        printf("%s: %d\n", __PRETTY_FUNCTION__, layer);
        for(size_t i = 0; i < surfaces.size(); i++)
            surfaces.itemAt(i)->set_layer(layer);
    }
};

struct SessionService : public ubuntu::ui::SessionService
{
    SessionService()
    {
    }

    const ubuntu::application::ui::Session::Ptr& start_a_new_session(const ubuntu::application::ui::SessionCredentials& cred)
    {
        (void) cred;
        static ubuntu::application::ui::Session::Ptr session(new Session(cred));
        return session;
    }
};

struct MockSetup : public ubuntu::application::ui::Setup
{
    ubuntu::application::ui::StageHint stage_hint()
    {
        return ubuntu::application::ui::main_stage;
    }

    ubuntu::application::ui::FormFactorHintFlags form_factor_hint()
    {
        return ubuntu::application::ui::desktop_form_factor;
    }
};

}

// We need to inject some platform specific symbols here.
namespace ubuntu
{
namespace application
{
namespace ui
{
/*const ubuntu::application::ui::SurfaceFactory::Ptr& ubuntu::application::ui::SurfaceFactory::instance()
{
    static ubuntu::application::ui::SurfaceFactory::Ptr session(new MockSurfaceFactory());
    return session;
    }*/

void init(int argc, char** argv)
{
    (void) argc;
    (void) argv;
}

const ubuntu::application::ui::Setup::Ptr& ubuntu::application::ui::Setup::instance()
{
    static ubuntu::application::ui::Setup::Ptr session(new android::MockSetup());
    return session;
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
}
}
