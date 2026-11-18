#pragma once

#include <DHT.h>

#include "./dht_sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class DHT22Sensor : public DHTSensor
        {
        public:
            DHT22Sensor(uint8_t pin) : DHTSensor(pin, DHT22) {}
        };
    }
}
