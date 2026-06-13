#pragma once

#include <device/wb_msw.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMSWHumidity : public Sensor
        {
        public:
            WBMSWHumidity(EDWB::MSW* msw) : _msw(msw) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "%";
                _config.deviceClass = EDHA::deviceClassSensorHumidity;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                auto result = _msw->getHumidity();

                return {result._value, result._success};
            }

        private:
            EDWB::MSW* _msw = nullptr;
        };
    }
}
