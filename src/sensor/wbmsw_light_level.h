#pragma once

#include <device/wb_msw.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMSWLightLevel : public Sensor
        {
        public:
            WBMSWLightLevel(EDWB::MSW* msw) : _msw(msw) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "lx";
                _config.deviceClass = "illuminance";

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                auto result = _msw->getCO2Value();

                return {result._value, result._success};
            }

        private:
            EDWB::MSW* _msw = nullptr;
        };
    }
}
