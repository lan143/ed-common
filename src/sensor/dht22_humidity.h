#pragma once

#include "./dht22_sensor.h"
#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class DHT22Humidity : public Sensor
        {
        public:
            DHT22Humidity(DHT22Sensor* dht) : _dht(dht) {}

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
            DHT22Sensor* _dht = nullptr;
        };
    }
}
