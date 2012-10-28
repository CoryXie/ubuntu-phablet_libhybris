#ifndef UBUNTU_APPLICATION_SENSORS_SENSOR_SERVICE_H_
#define UBUNTU_APPLICATION_SENSORS_SENSOR_SERVICE_H_

#include "ubuntu/platform/shared_ptr.h"

#include "ubuntu/application/sensors/sensor.h"

namespace ubuntu
{
namespace application
{
namespace sensors
{
class SensorService : public ubuntu::platform::ReferenceCountedBase
{
  public:
    static Sensor::Ptr sensor_for_type(SensorType type);
  protected:
    SensorService() {}
    virtual ~SensorService() {}

    SensorService(const SensorService&) = delete;
    SensorService& operator=(const SensorService&) = delete;
};
}
}
}

#endif // UBUNTU_APPLICATION_SENSORS_SENSOR_SERVICE_H_
