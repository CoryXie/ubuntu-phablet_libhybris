#ifndef HYBRIS_APPLICATION_MANAGER_H_
#define HYBRIS_APPLICATION_MANAGER_H_

#include <binder/IInterface.h>

namespace android
{

class IApplicationManagerSession : public IInterface
{
  public:
    DECLARE_META_INTERFACE(ApplicationManagerSession);

    virtual void raise_application_surfaces_to_layer(int layer) = 0;
    
  protected:
    enum
    {
        RAISE_APPLICATION_SURFACES_TO_LAYER_COMMAND = IBinder::FIRST_CALL_TRANSACTION
    };
};

class BnApplicationManagerSession : public BnInterface<IApplicationManagerSession>
{
  public:
    BnApplicationManagerSession();
    virtual ~BnApplicationManagerSession();
    
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);                                   
};

class BpApplicationManagerSession : public BpInterface<IApplicationManagerSession>
{
  public:
    BpApplicationManagerSession(const sp<IBinder>& impl);
    ~BpApplicationManagerSession();
    
    void raise_application_surfaces_to_layer(int layer);        
};

class IApplicationManager : public IInterface
{
  public:  
    DECLARE_META_INTERFACE(ApplicationManager);
  
    static const char* exported_service_name()
    {
        return "UbuntuApplicationManager";
    }

    virtual void start_a_new_session(const String8& app_name,
                                     const sp<IApplicationManagerSession>& session,
                                     int ashmem_fd,
                                     int out_socket_fd,
                                     int in_socket_fd) = 0;
  protected:
    enum
    {
        START_A_NEW_SESSION_COMMAND = IBinder::FIRST_CALL_TRANSACTION
    };
};

class BnApplicationManager : public BnInterface<IApplicationManager>
{
  public:
    BnApplicationManager();
    virtual ~BnApplicationManager();
    
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};

class BpApplicationManager : public BpInterface<IApplicationManager>
{
  public:
    BpApplicationManager(const sp<IBinder>& impl);
    ~BpApplicationManager();
    
    void start_a_new_session(const String8& app_name,
                             const sp<IApplicationManagerSession>& session,
                             int ashmem_fd,
                             int out_socket_fd,
                             int in_socket_fd);
};

}

#endif // HYBRIS_APPLICATION_MANAGER_H_
