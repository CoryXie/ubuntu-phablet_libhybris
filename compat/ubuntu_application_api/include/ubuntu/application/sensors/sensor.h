#ifndef UBUNTU_APPLICATION_SENSORS_SENSOR_H_
#define UBUNTU_APPLICATION_SENSORS_SENSOR_H_

#include "ubuntu/platform/shared_ptr.h"

#include "ubuntu/application/sensors/sensor_listener.h"
#include "ubuntu/application/sensors/sensor_type.h"

#include <cstdint>

namespace ubuntu
{
namespace application
{
namespace sensors
{

class Sensor : public ubuntu::platform::ReferenceCountedBase
{
  public:
    typedef ubuntu::platform::shared_ptr<Sensor> Ptr;    

    virtual const char* name() = 0;
    virtual const char* vendor() = 0;

    virtual int32_t id() = 0;

    virtual void register_listener(const SensorListener::Ptr& listener) = 0;

    virtual void enable() = 0;
    virtual void disable() = 0;

    virtual SensorType type() = 0;
    
    virtual float min_value() = 0;
    virtual float max_value() = 0;
    virtual float resolution() = 0;

    virtual float power_consumption() = 0;

    virtual int32_t min_delay() = 0;

  protected:
    Sensor() {}
    virtual ~Sensor() {}
    
    Sensor(const Sensor&) = delete;
    Sensor& operator=(const Sensor&) = delete;
};
}
}
}

#endif // UBUNTU_APPLICATION_SENSORS_SENSOR_H_
