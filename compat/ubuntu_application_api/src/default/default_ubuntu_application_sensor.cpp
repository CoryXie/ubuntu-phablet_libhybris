/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include <ubuntu/application/sensors/ubuntu_application_sensors.h>

#include <ubuntu/application/sensors/sensor.h>
#include <ubuntu/application/sensors/sensor_listener.h>
#include <ubuntu/application/sensors/sensor_service.h>
#include <ubuntu/application/sensors/sensor_type.h>

#include <cstdio>

namespace
{
struct SensorListener : public ubuntu::application::sensors::SensorListener
{
    SensorListener() : observer(NULL)
    {
    }

    void on_new_reading(const ubuntu::application::sensors::SensorReading::Ptr& reading)
    {
        if(!observer)
            return;

        if (!observer->on_new_accelerometer_reading_cb)
            return;

        static ubuntu_sensor_accelerometer_reading r;
        r.timestamp = reading->timestamp;
        r.acceleration_x = reading->acceleration[0];
        r.acceleration_y = reading->acceleration[1];
        r.acceleration_z = reading->acceleration[2];

        observer->on_new_accelerometer_reading_cb(&r, observer->context);
    }

    ubuntu_accelerometer_observer* observer;
};

ubuntu::application::sensors::Sensor::Ptr accelerometer;
ubuntu::application::sensors::SensorListener::Ptr accelerometer_listener;
}


void ubuntu_sensor_install_accelerometer_observer(ubuntu_accelerometer_observer* observer)
{
    if (accelerometer == NULL)
    {

        accelerometer =
            ubuntu::application::sensors::SensorService::sensor_for_type(
                ubuntu::application::sensors::sensor_type_accelerometer);

        SensorListener* sl = new SensorListener();
        sl->observer = observer;

        accelerometer_listener = sl;
        accelerometer->register_listener(accelerometer_listener);
        accelerometer->enable();
    }
}
