#pragma once

#include <Arduino.h>
#include <device/mtd262mb.h>

#include "./binary_sensor.h"

namespace EDCommon
{
    namespace BinarySensor
    {
        class MTD262MB : public BinarySensor
        {
        public:
            MTD262MB(EDWB::MTD262MB* mtd262mb) : _mtd262mb(mtd262mb) {}

        protected:
            bool preInit() override
            {
                _config.deviceClass = "occupancy";
                _config.reverse = false;

                return true;
            }

            std::pair<bool, bool> isActiveInternal() override
            {
                return _mtd262mb->isHumanDetected();
            }

        private:
            EDWB::MTD262MB* _mtd262mb = nullptr;
        };
    }
}
