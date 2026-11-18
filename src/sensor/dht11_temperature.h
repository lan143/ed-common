#pragma once

#include "./dht11_sensor.h"
#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class DHT11Temperature : public Sensor
        {
        public:
            DHT11Temperature(DHT11Sensor* dht) : _dht(dht) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "°C";
                _config.deviceClass = EDHA::deviceClassSensorTemperature;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                return _dht->getTemperature();
            }

        private:
            DHT11Sensor* _dht = nullptr;
        };
    }
}
