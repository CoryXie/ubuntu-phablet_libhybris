#ifndef WELL_KNOWN_APPLICATION_REGISTRY_H_
#define WELL_KNOWN_APPLICATION_REGISTRY_H_

#include <ubuntu/ui/well_known_applications.h>

#include <utils/KeyedVector.h>
#include <utils/String8.h>

namespace mir
{
class WellKnownApplicationRegistry
{
  public:
    static ubuntu::ui::WellKnownApplication type_for_desktop_file(const android::String8& file)
    {
        static android::KeyedVector<android::String8, ubuntu::ui::WellKnownApplication> lut = init_forward_lut();

        ssize_t idx = lut.indexOfKey(file);
        if (idx >= 0 && idx < lut.size())
            return lut.valueAt(idx);

        return ubuntu::ui::unknown_app;
    }

    static android::String8 desktop_file_for_type(ubuntu::ui::WellKnownApplication app)
    {
        static android::KeyedVector<ubuntu::ui::WellKnownApplication, android::String8> lut = init_backward_lut();

        return lut.valueFor(app);
    }

    void register_application_instance_for_type(ubuntu::ui::WellKnownApplication app,
                                                const android::sp<android::IBinder>& inst)
    {
        if (has_instance_for_type(app))
            return;

        registry.add(app, inst);
    }

    void unregister_application_instance_for_type(ubuntu::ui::WellKnownApplication app)
    {
        if (!has_instance_for_type(app))
            return;

        registry.removeItem(app);
    }

    bool has_instance_for_type(ubuntu::ui::WellKnownApplication app) const
    {
        ssize_t idx = registry.indexOfKey(app);

        return idx >= 0 && idx < registry.size();
    }

    android::sp<android::IBinder> instance_for_type(ubuntu::ui::WellKnownApplication app)
    {
        static android::sp<android::IBinder> unknown_instance;

        if (!has_instance_for_type(app))
            return unknown_instance;

        return registry.valueFor(app);
    }

  private:
    static android::KeyedVector<android::String8, ubuntu::ui::WellKnownApplication> init_forward_lut()
    {
        android::KeyedVector<android::String8, ubuntu::ui::WellKnownApplication> result;
        result.add(android::String8("/usr/share/applications/gallery.desktop"), ubuntu::ui::gallery_app);
        result.add(android::String8("/usr/share/applications/camera.desktop"), ubuntu::ui::camera_app);
        result.add(android::String8("/usr/share/applications/snowshoe.desktop"), ubuntu::ui::browser_app);
        return result;
    }

    static android::KeyedVector<ubuntu::ui::WellKnownApplication, android::String8> init_backward_lut()
    {
        android::KeyedVector<ubuntu::ui::WellKnownApplication, android::String8> result;
        result.add(ubuntu::ui::gallery_app, android::String8("/usr/share/applications/gallery.desktop"));
        result.add(ubuntu::ui::camera_app, android::String8("/usr/share/applications/camera.desktop"));
        result.add(ubuntu::ui::browser_app, android::String8("/usr/share/applications/snowshoe.desktop"));
        return result;
    }

    android::KeyedVector< ubuntu::ui::WellKnownApplication, android::sp<android::IBinder> > registry;
};
}

#endif // WELL_KNOWN_APPLICATION_REGISTRY_H_
