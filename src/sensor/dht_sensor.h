#pragma once

#include <Arduino.h>
#include <DHT.h>

namespace EDCommon
{
    namespace Sensor
    {
        class DHTSensor
        {
        public:
            DHTSensor(uint8_t pin, uint8_t type);

            bool begin();
            std::pair<float_t, bool> getTemperature();
            std::pair<float_t, bool> getHumidity();

        private:
            DHT _dht;
        };
    }
}
