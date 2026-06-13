#pragma once

#include <device/wb_msw.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMSWCO2 : public Sensor
        {
        public:
            WBMSWCO2(EDWB::MSW* msw) : _msw(msw) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "ppm";
                _config.deviceClass = "carbon_dioxide";

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
