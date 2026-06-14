#pragma once

#include <device/wb_ms.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMSHumidity : public Sensor
        {
        public:
            WBMSHumidity(EDWB::MS* msw) : _msw(msw) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "%";
                _config.deviceClass = EDHA::deviceClassSensorHumidity;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                return _msw->getHumidity();
            }

        private:
            EDWB::MS* _msw = nullptr;
        };
    }
}
