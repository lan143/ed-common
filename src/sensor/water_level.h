#pragma once

#include <Arduino.h>
#include <device/qdy30a.h>
#include <discovery.h>
#include <mqtt.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WaterLevel : public Sensor
        {
        public:
            WaterLevel(EDWB::QDY30A* qdy30a) : _qdy30a(qdy30a) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "m";

                return true;
            }

            bool postInit() override
            {
                if (!loadConstants()) {
                    LOGE("postInit", "failed to load constants");
                }

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override;

        private:
            bool loadConstants();

        private:
            EDWB::QDY30A* _qdy30a = nullptr;

        private:
            int32_t _unitOfMeasurement = 0;
            int32_t _dotPosition = 0;
            bool _constantsLoaded = false;
        };
    }
}