#pragma once

#include <device/wb_ms.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMSOneWireTemperature : public Sensor
        {
        public:
            WBMSOneWireTemperature(uint8_t channel, EDWB::MS* msw) : _msw(msw), _channel(channel) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "°C";
                _config.deviceClass = EDHA::deviceClassSensorTemperature;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                return _msw->getOneWireTemperature(_channel);
            }

        private:
            EDWB::MS* _msw = nullptr;
            uint8_t _channel = 0;
        };
    }
}
