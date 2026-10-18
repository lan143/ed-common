#pragma once

#include <Arduino.h>
#include <device/wb_mai6.h>
#include <discovery.h>
#include <mqtt.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMAI6Pressure : public Sensor
        {
        public:
            WBMAI6Pressure(
                EDWB::MAI6* device,
                uint8_t channel,
                float_t maxPhysicalValue = 10.0f,
                uint16_t sensorType = 0x1302,
                uint16_t minCalculatedValue = 0,
                uint16_t maxCalculatedValue = 1000
            )
                : _device(device)
                , _channel(channel)
                , _maxPhysicalValue(maxPhysicalValue)
                , _sensorType(sensorType)
                , _minCalculatedValue(minCalculatedValue)
                , _maxCalculatedValue(maxCalculatedValue)
            {
            }

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "bar";
                _config.deviceClass = "pressure";

                return true;
            }

            bool postInit() override
            {
                if (!setupChannel()) {
                    LOGE("postInit", "failed to setup pressure channel");
                }

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override;

        private:
            bool setupChannel();

        private:
            EDWB::MAI6* _device = nullptr;

        private:
            uint8_t _channel = 0;
            float_t _maxPhysicalValue = 10.0f;
            uint16_t _sensorType = 0x1302;
            uint16_t _minCalculatedValue = 0;
            uint16_t _maxCalculatedValue = 1000;
        };
    }
}
