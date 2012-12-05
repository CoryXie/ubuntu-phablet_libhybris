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
#ifndef UBUNTU_APPLICATION_SENSORS_C_API_H_
#define UBUNTU_APPLICATION_SENSORS_C_API_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct
    {
        int64_t timestamp;

        float acceleration_x;
        float acceleration_y;
        float acceleration_z;
    } ubuntu_sensor_accelerometer_reading;

    typedef struct
    {
        int64_t timestamp;

        float distance;
    } ubuntu_sensor_proximity_reading;

    typedef struct
    {
        int64_t timestamp;
        
        float light;
    } ubuntu_sensor_ambient_light_reading;

    typedef struct
    {
        typedef void (*on_new_accelerometer_reading)(ubuntu_sensor_accelerometer_reading* reading, void* context);
        typedef void (*on_new_proximity_reading)(ubuntu_sensor_proximity_reading* reading, void* context);
        typedef void (*on_new_ambient_light_reading)(ubuntu_sensor_ambient_light_reading* reading, void* context);

        on_new_accelerometer_reading on_new_accelerometer_reading_cb;
        on_new_proximity_reading on_new_proximity_reading_cb;
        on_new_ambient_light_reading on_new_ambient_light_reading_cb;
        
        void* context;
    } ubuntu_sensor_observer;

    void ubuntu_sensor_install_observer(ubuntu_sensor_observer* observer);

#ifdef __cplusplus
}
#endif

#endif // UBUNTU_APPLICATION_SENSORS_C_API_H_
