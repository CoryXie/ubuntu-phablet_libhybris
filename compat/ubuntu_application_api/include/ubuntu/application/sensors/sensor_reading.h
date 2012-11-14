#ifndef UBUNTU_APPLICATION_SENSORS_SENSOR_READING_H_
#define UBUNTU_APPLICATION_SENSORS_SENSOR_READING_H_

#include "ubuntu/platform/shared_ptr.h"

#include <cstddef>
#include <cstdint>

namespace ubuntu
{
namespace application
{
namespace sensors
{

template<size_t size, typename NumericType = float>
struct Vector
{
    NumericType v[size];
    
    NumericType& operator[](size_t index)
    {
        return v[index];
    }

    const NumericType& operator[](size_t index) const
    {
        return v[index];
    }
};

struct SensorReading : public ubuntu::platform::ReferenceCountedBase
{
    typedef ubuntu::platform::shared_ptr<SensorReading> Ptr;

    SensorReading() : timestamp(-1)
    {
    }
    
    virtual ~SensorReading()
    {
    }
    
    int64_t timestamp;
    union
    {
        Vector<3> vector;
        Vector<3> acceleration;
        Vector<3> magnetic;
        float temperature;
        float distance;
        float light;
        float pressure;
    };
};
}
}
}

#endif // UBUNTU_APPLICATION_SENSORS_SENSOR_READING_H_
