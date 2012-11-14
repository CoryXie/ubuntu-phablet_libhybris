#ifndef UBUNTU_APPLICATION_SENSORS_SENSOR_TYPE_H_
#define UBUNTU_APPLICATION_SENSORS_SENSOR_TYPE_H_

namespace ubuntu
{
namespace application
{
namespace sensors
{
enum SensorType
{
    first_defined_sensor_type = 0,
    sensor_type_accelerometer = first_defined_sensor_type,
    sensor_type_magnetic_field,
    sensor_type_gyroscope,
    sensor_type_light,
    sensor_type_proximity,
    sensor_type_orientation,
    sensor_type_linear_acceleration,
    sensor_type_rotation_vector,
    undefined_sensor_type
};
}
}
}

#endif // UBUNTU_APPLICATION_SENSORS_SENSOR_TYPE_H_
