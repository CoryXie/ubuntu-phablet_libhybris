#ifndef HYBRIS_APPLICATION_MANAGER_H_
#define HYBRIS_APPLICATION_MANAGER_H_

#include <binder/IInterface.h>

namespace android
{

class IApplicationManagerSession : public IInterface
{
  public:
    DECLARE_META_INTERFACE(ApplicationManagerSession);

    struct SurfaceProperties
    {
        int32_t layer;
        int32_t left;
        int32_t top;
        int32_t right;
        int32_t bottom;
    };

    virtual void raise_application_surfaces_to_layer(int layer) = 0;
    virtual SurfaceProperties query_surface_properties_for_token(int32_t token) = 0;

  protected:
    enum
    {
        RAISE_APPLICATION_SURFACES_TO_LAYER_COMMAND = IBinder::FIRST_CALL_TRANSACTION,
        QUERY_SURFACE_PROPERTIES_FOR_TOKEN_COMMAND
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
    IApplicationManagerSession::SurfaceProperties query_surface_properties_for_token(int32_t token);
};

class IApplicationManagerObserver : public IInterface
{
  public:
    DECLARE_META_INTERFACE(ApplicationManagerObserver);
    
    virtual void on_session_born(int id,
                                 const String8& desktop_file) = 0;

    virtual void on_session_focused(int id, 
                                    const String8& desktop_file) = 0;

    virtual void on_session_died(int id,
                                 const String8& desktop_file) = 0;

  protected:
    enum
    {
        ON_SESSION_BORN_NOTIFICATION = IBinder::FIRST_CALL_TRANSACTION,
        ON_SESSION_FOCUSED_NOTIFICATION,
        ON_SESSION_DIED_NOTIFICATION
    };

    IApplicationManagerObserver(const IApplicationManagerObserver&) = delete;
    IApplicationManagerObserver& operator=(const IApplicationManagerObserver&) = delete;
};

class BnApplicationManagerObserver : public BnInterface<IApplicationManagerObserver>
{
  public:        
    status_t onTransact(uint32_t code,
                        const Parcel& data,
                        Parcel* reply,
                        uint32_t flags = 0);
};

class BpApplicationManagerObserver : public BpInterface<IApplicationManagerObserver>
{
  public:
    BpApplicationManagerObserver(const sp<IBinder>& impl);

    void on_session_born(int id,
                         const String8& desktop_file);
    
    void on_session_focused(int id, 
                            const String8& desktop_file);
    
    void on_session_died(int id,
                         const String8& desktop_file);
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
                                     const String8& desktop_file,
                                     const sp<IApplicationManagerSession>& session,
                                     int ashmem_fd,
                                     int out_socket_fd,
                                     int in_socket_fd) = 0;

    virtual void register_a_surface(const String8& title,
                                    const sp<IApplicationManagerSession>& session,
                                    int32_t token,
                                    int ashmem_fd,
                                    int out_socket_fd,
                                    int in_socket_fd) = 0;

    virtual void register_an_observer(const sp<IApplicationManagerObserver>& observer) = 0;

  protected:
    enum
    {
        START_A_NEW_SESSION_COMMAND = IBinder::FIRST_CALL_TRANSACTION,
        REGISTER_A_SURFACE_COMMAND,
        REGISTER_AN_OBSERVER_COMMAND
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
                             const String8& desktop_file,
                             const sp<IApplicationManagerSession>& session,
                             int ashmem_fd,
                             int out_socket_fd,
                             int in_socket_fd);

    void register_a_surface(const String8& title,
                            const android::sp<android::IApplicationManagerSession>& session,
                            int32_t token,
                            int ashmem_fd,
                            int out_socket_fd,
                            int in_socket_fd);

    void register_an_observer(const sp<IApplicationManagerObserver>& observer);
};

}

#endif // HYBRIS_APPLICATION_MANAGER_H_
