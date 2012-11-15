#include "application_manager.h"

#include <binder/Parcel.h>
#include <utils/String8.h>

namespace android
{
IMPLEMENT_META_INTERFACE(ApplicationManagerObserver, "UbuntuApplicationManagerObserver");
IMPLEMENT_META_INTERFACE(ApplicationManagerSession, "UbuntuApplicationManagerSession");
IMPLEMENT_META_INTERFACE(ApplicationManager, "UbuntuApplicationManager");

BnApplicationManagerSession::BnApplicationManagerSession()
{
}

BnApplicationManagerSession::~BnApplicationManagerSession() {}

status_t BnApplicationManagerSession::onTransact(uint32_t code,
                                                 const Parcel& data,
                                                 Parcel* reply,
                                                 uint32_t flags)
{
    switch(code)
    {
        case RAISE_APPLICATION_SURFACES_TO_LAYER_COMMAND:
            {
                int32_t layer;
                data.readInt32(&layer);
                
                raise_application_surfaces_to_layer(layer);
            }
            break;
        case QUERY_SURFACE_PROPERTIES_FOR_TOKEN_COMMAND:
            {
                int32_t token = data.readInt32();
                IApplicationManagerSession::SurfaceProperties props = 
                        query_surface_properties_for_token(token);
                reply->writeInt32(props.layer);
                reply->writeInt32(props.left);
                reply->writeInt32(props.top);
                reply->writeInt32(props.right);
                reply->writeInt32(props.bottom);
            }
    }
    return NO_ERROR;
}

BpApplicationManagerSession::BpApplicationManagerSession(const sp<IBinder>& impl)
        : BpInterface<IApplicationManagerSession>(impl)
{
}

BpApplicationManagerSession::~BpApplicationManagerSession()
{
}

void BpApplicationManagerSession::raise_application_surfaces_to_layer(int layer)
{
    Parcel in, out;
    in.writeInt32(layer);

    remote()->transact(
        RAISE_APPLICATION_SURFACES_TO_LAYER_COMMAND,
        in,
        &out);              
}

IApplicationManagerSession::SurfaceProperties BpApplicationManagerSession::query_surface_properties_for_token(int32_t token)
{
    Parcel in, out;
    in.writeInt32(token);

    remote()->transact(
        QUERY_SURFACE_PROPERTIES_FOR_TOKEN_COMMAND,
        in,
        &out);
    
    IApplicationManagerSession::SurfaceProperties props;
    props.layer = out.readInt32();
    props.left = out.readInt32();
    props.top = out.readInt32();
    props.right = out.readInt32();
    props.bottom = out.readInt32();

    return props;
}

status_t BnApplicationManagerObserver::onTransact(uint32_t code,
                                                  const Parcel& data,
                                                  Parcel* reply,
                                                  uint32_t flags)
{
    int id = data.readInt32();
    String8 desktop_file = data.readString8();

    switch(code)
    {
        case ON_SESSION_BORN_NOTIFICATION:
            on_session_born(id, desktop_file);
            break;

        case ON_SESSION_FOCUSED_NOTIFICATION:
            on_session_focused(id, desktop_file);
            break;

        case ON_SESSION_DIED_NOTIFICATION:
            on_session_died(id, desktop_file);
            break;
    }

    return NO_ERROR;
}

BpApplicationManagerObserver::BpApplicationManagerObserver(const sp<IBinder>& impl)
        : BpInterface<IApplicationManagerObserver>(impl)
{
    
}
    
void BpApplicationManagerObserver::on_session_born(int id,
                                                   const String8& desktop_file_hint)
{
    Parcel in, out;
    in.writeInt32(id);
    in.writeString8(desktop_file_hint);

    remote()->transact(
        ON_SESSION_BORN_NOTIFICATION,
        in,
        &out,
        android::IBinder::FLAG_ONEWAY);
}

void BpApplicationManagerObserver::on_session_focused(int id,
                                                      const String8& desktop_file_hint)
{
    Parcel in, out;
    in.writeInt32(id);
    in.writeString8(desktop_file_hint);

    remote()->transact(
        ON_SESSION_FOCUSED_NOTIFICATION,
        in,
        &out,
        android::IBinder::FLAG_ONEWAY);
}

void BpApplicationManagerObserver::on_session_died(int id,
                                                   const String8& desktop_file_hint)
{
    Parcel in, out;
    in.writeInt32(id);
    in.writeString8(desktop_file_hint);

    remote()->transact(
        ON_SESSION_FOCUSED_NOTIFICATION,
        in,
        &out,
        android::IBinder::FLAG_ONEWAY);
}

BnApplicationManager::BnApplicationManager() 
{
}

BnApplicationManager::~BnApplicationManager() 
{
}

status_t BnApplicationManager::onTransact(uint32_t code,
                                          const Parcel& data,
                                          Parcel* reply,
                                          uint32_t flags)
{
    switch(code)
    {
        case START_A_NEW_SESSION_COMMAND:
            {
                String8 app_name = data.readString8();
                sp<IBinder> binder = data.readStrongBinder();
                sp<BpApplicationManagerSession> session(new BpApplicationManagerSession(binder));
                int ashmem_fd = data.readFileDescriptor();
                int out_fd = data.readFileDescriptor();
                int in_fd = data.readFileDescriptor();
                
                start_a_new_session(app_name, session, ashmem_fd, out_fd, in_fd);
            }
            break;
        case REGISTER_A_SURFACE_COMMAND:
            {
                String8 title = data.readString8();
                sp<IBinder> binder = data.readStrongBinder();
                sp<BpApplicationManagerSession> session(new BpApplicationManagerSession(binder));
                int32_t surface_token = data.readInt32();
                int ashmem_fd = data.readFileDescriptor();
                int out_fd = data.readFileDescriptor();
                int in_fd = data.readFileDescriptor();

                register_a_surface(title, session, surface_token, ashmem_fd, out_fd, in_fd);
            }
            break;
        case REGISTER_AN_OBSERVER_COMMAND:
            sp<IBinder> binder = data.readStrongBinder();
            sp<BpApplicationManagerObserver> observer(new BpApplicationManagerObserver(binder));
            register_an_observer(observer);
    }
    return NO_ERROR;
}

BpApplicationManager::BpApplicationManager(const sp<IBinder>& impl) 
        : BpInterface<IApplicationManager>(impl) 
{
}

BpApplicationManager::~BpApplicationManager() 
{
}

void BpApplicationManager::start_a_new_session(const String8& app_name,
                                               const sp<IApplicationManagerSession>& session,
                                               int ashmem_fd,
                                               int out_socket_fd,
                                               int in_socket_fd)
{
    //printf("%s \n", __PRETTY_FUNCTION__);
    Parcel in, out;
    in.pushAllowFds(true);
    in.writeString8(app_name);
    in.writeStrongBinder(session->asBinder());
    in.writeFileDescriptor(ashmem_fd);
    in.writeFileDescriptor(out_socket_fd);
    in.writeFileDescriptor(in_socket_fd);

    remote()->transact(START_A_NEW_SESSION_COMMAND,
               in,
               &out);
}

void BpApplicationManager::register_a_surface(const String8& title,
                                              const sp<IApplicationManagerSession>& session,
                                              int32_t token,
                                              int ashmem_fd,
                                              int out_socket_fd,
                                              int in_socket_fd)
{
    //printf("%s \n", __PRETTY_FUNCTION__);
    Parcel in, out;
    in.pushAllowFds(true);
    in.writeString8(title);
    in.writeStrongBinder(session->asBinder());
    in.writeInt32(token);
    in.writeFileDescriptor(ashmem_fd);
    in.writeFileDescriptor(out_socket_fd);
    in.writeFileDescriptor(in_socket_fd);

    remote()->transact(REGISTER_A_SURFACE_COMMAND,
               in,
               &out);
}

void BpApplicationManager::register_an_observer(const sp<IApplicationManagerObserver>& observer)
{
    Parcel in, out;
    in.writeStrongBinder(observer->asBinder());

    remote()->transact(REGISTER_AN_OBSERVER_COMMAND,
                       in,
                       &out);
}

}
