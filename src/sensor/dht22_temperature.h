#pragma once

#include "./dht22_sensor.h"
#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class DHT22Temperature : public Sensor
        {
        public:
            DHT22Temperature(DHT22Sensor* dht) : _dht(dht) {}

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
            DHT22Sensor* _dht = nullptr;
        };
    }
}
