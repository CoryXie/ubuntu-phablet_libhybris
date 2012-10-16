#include "application_manager.h"

#include <binder/Parcel.h>
#include <utils/String8.h>

namespace android
{
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
    int32_t layer;
    data.readInt32(&layer);

    raise_application_surfaces_to_layer(layer);

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
    String8 app_name = data.readString8();
    sp<IBinder> binder = data.readStrongBinder();
    sp<BpApplicationManagerSession> session(new BpApplicationManagerSession(binder));
    int ashmem_fd = data.readFileDescriptor();
    int out_fd = data.readFileDescriptor();
    int in_fd = data.readFileDescriptor();

    start_a_new_session(app_name, session, ashmem_fd, out_fd, in_fd);
    
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
    printf("%s \n", __PRETTY_FUNCTION__);
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
}
