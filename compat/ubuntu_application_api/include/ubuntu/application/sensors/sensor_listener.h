#ifndef UBUNTU_APPLICATION_SENSORS_SENSOR_LISTENER_H_
#define UBUNTU_APPLICATION_SENSORS_SENSOR_LISTENER_H_

#include "ubuntu/platform/shared_ptr.h"

#include "ubuntu/application/sensors/sensor_reading.h"

namespace ubuntu
{
namespace application
{
namespace sensors
{
class SensorListener : public ubuntu::platform::ReferenceCountedBase
{
  public:
    typedef ubuntu::platform::shared_ptr<SensorListener> Ptr;

    virtual void on_new_reading(const SensorReading::Ptr& reading) = 0;

  protected:
    SensorListener() {}
    virtual ~SensorListener() {}

    SensorListener(const SensorListener&) = delete;
    SensorListener& operator=(const SensorListener&) = delete;
};
}
}
}

#endif // UBUNTU_APPLICATION_SENSORS_SENSOR_LISTENER_H_
