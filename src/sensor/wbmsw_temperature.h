#pragma once

#include <device/wb_msw.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMSWTemperature : public Sensor
        {
        public:
            WBMSWTemperature(EDWB::MSW* msw) : _msw(msw) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "°C";
                _config.deviceClass = EDHA::deviceClassSensorTemperature;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                auto result = _msw->getTemperature();

                return {result._value, result._success};
            }

        private:
            EDWB::MSW* _msw = nullptr;
        };
    }
}
