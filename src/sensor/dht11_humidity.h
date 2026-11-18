#pragma once

#include "./dht11_sensor.h"
#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class DHT11Humidity : public Sensor
        {
        public:
            DHT11Humidity(DHT11Sensor* dht) : _dht(dht) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "%";
                _config.deviceClass = EDHA::deviceClassSensorHumidity;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                return _dht->getHumidity();
            }

        private:
            DHT11Sensor* _dht = nullptr;
        };
    }
}
