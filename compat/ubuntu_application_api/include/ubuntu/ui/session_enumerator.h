#ifndef UBUNTU_UI_SESSION_ENUMERATOR_H_
#define UBUNTU_UI_SESSION_ENUMERATOR_H_

#include "ubuntu/platform/shared_ptr.h"

#include <GLES2/gl2.h>

namespace ubuntu
{
namespace ui
{

class SessionProperties : public platform::ReferenceCountedBase
{
  public:
    static const char* key_application_instance_id();
    static const char* key_application_name();
    static const char* key_desktop_file_hint();

    typedef platform::shared_ptr<SessionProperties> Ptr;

    virtual const char* value_for_key(const char* key) const = 0;

    const char* application_instance_id() const
    {
        return value_for_key(SessionProperties::key_application_instance_id());
    }

    const char* application_name() const
    {
        return value_for_key(SessionProperties::key_application_name());
    }

    const char* desktop_file_hint() const
    {
        return value_for_key(SessionProperties::key_desktop_file_hint());
    }

  protected:
    SessionProperties() {}
    virtual ~SessionProperties() {}

    SessionProperties(const SessionProperties&) = delete;
    SessionProperties& operator=(const SessionProperties&) = delete;
};

class SessionLifeCycleObserver : public platform::ReferenceCountedBase
{
  public:
    typedef platform::shared_ptr<SessionLifeCycleObserver> Ptr;

    virtual void on_application_born(const SessionProperties::Ptr& props) = 0;
    
    virtual void on_application_died(const SessionProperties::Ptr& props) = 0;

  protected:
    SessionLifeCycleObserver() {}
    virtual ~SessionLifeCycleObserver() {}
    
    SessionLifeCycleObserver(const SessionLifeCycleObserver&) = delete;
    SessionLifeCycleObserver& operator=(const SessionLifeCycleObserver&) = delete;
};

class SessionPreviewProvider : public platform::ReferenceCountedBase
{
  public:
    typedef platform::shared_ptr<SessionPreviewProvider> Ptr;

    virtual bool get_or_update_session_preview(GLuint texture, unsigned int& width, unsigned int& height) = 0;

  protected:
    SessionPreviewProvider() {}
    virtual ~SessionPreviewProvider() {}

    SessionPreviewProvider(const SessionPreviewProvider&) = delete;
    SessionPreviewProvider& operator=(const SessionPreviewProvider&) = delete;
};

class SessionEnumerator : public platform::ReferenceCountedBase
{
  public:
    typedef platform::shared_ptr<SessionEnumerator> Ptr;
    
    virtual void init_with_total_session_count(unsigned int session_count) = 0;

    virtual void for_each_session(const SessionProperties::Ptr& props, const SessionPreviewProvider::Ptr& preview_provider) = 0;

  protected:
    SessionEnumerator() {}
    virtual ~SessionEnumerator() {}

    SessionEnumerator(const SessionEnumerator&) = delete;
    SessionEnumerator& operator=(const SessionEnumerator&) = delete;
};
}
}

#endif // UBUNTU_UI_SESSION_ENUMERATOR_H_
