#pragma once

#include <Arduino.h>
#include <DHT.h>

namespace EDCommon
{
    namespace Sensor
    {
        class DHT22Sensor
        {
        public:
            DHT22Sensor(uint8_t pin);

            bool begin();
            std::pair<float_t, bool> getTemperature();
            std::pair<float_t, bool> getHumidity();

        private:
            DHT _dht;
        };
    }
}
