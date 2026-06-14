#pragma once

#include <device/wb_ms.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMSLightLevel : public Sensor
        {
        public:
            WBMSLightLevel(EDWB::MS* msw) : _msw(msw) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "lx";
                _config.deviceClass = "illuminance";

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                return _msw->getLightLevel();
            }

        private:
            EDWB::MS* _msw = nullptr;
        };
    }
}
