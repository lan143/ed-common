#pragma once

#include <Arduino.h>
#include <device/wb_msw.h>

#include "./binary_sensor.h"

namespace EDCommon
{
    namespace BinarySensor
    {
        class WBMSW : public BinarySensor
        {
        public:
            WBMSW(EDWB::MSW* msw) : _msw(msw) {}

        protected:
            bool preInit() override
            {
                _config.deviceClass = "motion";
                _config.reverse = false;

                return true;
            }

            std::pair<bool, bool> isActiveInternal() override
            {
                auto result = _msw->getMotion();

                return {result._success && result._value != 0, result._success};
            }

        private:
            EDWB::MSW* _msw = nullptr;
        };
    }
}
