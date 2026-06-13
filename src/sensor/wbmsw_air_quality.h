#pragma once

#include <device/wb_msw.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMSWAirQuality : public Sensor
        {
        public:
            WBMSWAirQuality(EDWB::MSW* msw) : _msw(msw) {}

        protected:
            bool preInit() override
            {
                _config.deviceClass = EDHA::deviceClassSensorAQI;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                auto result = _msw->getAirQuality();

                return {result._value, result._success};
            }

        private:
            EDWB::MSW* _msw = nullptr;
        };
    }
}
