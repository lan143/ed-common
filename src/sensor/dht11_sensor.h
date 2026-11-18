#pragma once

#include <DHT.h>

#include "./dht_sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class DHT11Sensor : public DHTSensor
        {
        public:
            DHT11Sensor(uint8_t pin) : DHTSensor(pin, DHT11) {}
        };
    }
}
