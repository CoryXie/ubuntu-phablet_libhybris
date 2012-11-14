#include <ubuntu/ui/ubuntu_ui_session_service.h>

#include <ubuntu/ui/session_service.h>
#include <ubuntu/ui/session_enumerator.h>

namespace
{
struct SessionLifeCycleObserver : public ubuntu::ui::SessionLifeCycleObserver
{
    SessionLifeCycleObserver(ubuntu_ui_session_lifecycle_observer* observer) : observer(observer)
    {
    }

    void on_application_born(const ubuntu::ui::SessionProperties::Ptr& props)
    {
        if (!observer)
            return;

        if (!observer->on_application_born)
            return;

        observer->on_application_born(&props, observer->context);
    }

    void on_application_died(const ubuntu::ui::SessionProperties::Ptr& props)
    {
        if (!observer)
            return;

        if (!observer->on_application_died)
            return;

        observer->on_application_died(&props, observer->context);
    }

    ubuntu_ui_session_lifecycle_observer* observer;
};

struct SessionEnumerator : public ubuntu::ui::SessionEnumerator
{
    SessionEnumerator(ubuntu_ui_session_enumerator* enumerator) : enumerator(enumerator)
    {
    }
    
    void init_with_total_session_count(unsigned int count)
    {
        if (!enumerator)
            return;

        if (!enumerator->init_with_total_session_count)
            return;

        enumerator->init_with_total_session_count(count, enumerator->context);
    }

    void for_each_session(const ubuntu::ui::SessionProperties::Ptr& props, const ubuntu::ui::SessionPreviewProvider::Ptr& preview_provider)
    {
        if (!enumerator)
            return;

        if (!enumerator->for_each_session)
            return;

        enumerator->for_each_session(&props, &preview_provider, enumerator->context);
    }

    ubuntu_ui_session_enumerator* enumerator;
};

}

const char* ubuntu_ui_session_properties_get_value_for_key(ubuntu_ui_session_properties props, const char* key)
{
    if (!props)
        return NULL;

    if (!key)
        return NULL;
    
    const ubuntu::ui::SessionProperties::Ptr* p = static_cast<const ubuntu::ui::SessionProperties::Ptr*>(props);

    return (*p)->value_for_key(key);
}

const char* ubuntu_ui_session_properties_get_application_instance_id(ubuntu_ui_session_properties props)
{
    if (!props)
        return NULL;

    const ubuntu::ui::SessionProperties::Ptr* p = static_cast<const ubuntu::ui::SessionProperties::Ptr*>(props);

    return (*p)->application_instance_id();
}

const char* ubuntu_ui_session_properties_get_application_name(ubuntu_ui_session_properties props)
{
    if (!props)
        return NULL;

    const ubuntu::ui::SessionProperties::Ptr* p = static_cast<const ubuntu::ui::SessionProperties::Ptr*>(props);

    return (*p)->application_name();
}

const char* ubuntu_ui_session_properties_get_desktop_file_hint(ubuntu_ui_session_properties props)
{
    if (!props)
        return NULL;

    const ubuntu::ui::SessionProperties::Ptr* p = static_cast<const ubuntu::ui::SessionProperties::Ptr*>(props);

    return (*p)->desktop_file_hint();
}

bool ubuntu_ui_session_preview_provider_update_session_preview_texture(ubuntu_ui_session_preview_provider pp, GLuint texture, unsigned int* width, unsigned int* height)
{
    if (!pp)
        return false;

    const ubuntu::ui::SessionPreviewProvider::Ptr* spp =
            static_cast<const ubuntu::ui::SessionPreviewProvider::Ptr*>(pp);

    return (*spp)->get_or_update_session_preview(texture, *width, *height);
}

void ubuntu_ui_session_install_session_lifecycle_observer(ubuntu_ui_session_lifecycle_observer* observer)
{
    ubuntu::ui::SessionLifeCycleObserver::Ptr p(new SessionLifeCycleObserver(observer));
    ubuntu::ui::SessionService::instance()->install_session_lifecycle_observer(p);
}

void ubuntu_ui_session_enumerate_running_sessions(ubuntu_ui_session_enumerator* enumerator){
    ubuntu::ui::SessionEnumerator::Ptr p(new SessionEnumerator(enumerator));
    ubuntu::ui::SessionService::instance()->enumerate_running_sessions(p);
}
