#pragma once

#include <device/wb_ms.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMSTemperature : public Sensor
        {
        public:
            WBMSTemperature(EDWB::MS* msw) : _msw(msw) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "°C";
                _config.deviceClass = EDHA::deviceClassSensorTemperature;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                return _msw->getTemperature();
            }

        private:
            EDWB::MS* _msw = nullptr;
        };
    }
}
