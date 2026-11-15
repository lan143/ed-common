#pragma once

#include <Arduino.h>
#include <PCF8574.h>

#include "./binary_sensor.h"

namespace EDCommon
{
    namespace BinarySensor
    {
        class PCF8574P : public BinarySensor
        {
        public:
            PCF8574P(
                uint8_t channel,
                EDHA::DeviceClass* deviceClass,
                bool reverse,
                PCF8574* driver,
                uint16_t debounceMs = 0
            ) : _channel(channel),
                _deviceClass(deviceClass),
                _reverse(reverse),
                _driver(driver),
                _debounceMs(debounceMs) {}

        protected:
            bool preInit() override;
            std::pair<bool, bool> isActiveInternal() override;

        private:
            uint8_t _channel = 0;
            EDHA::DeviceClass* _deviceClass = nullptr;
            bool _reverse = false;
            PCF8574* _driver = nullptr;
            uint16_t _debounceMs = 0;

        private:
            uint8_t _lastRaw = 0;
            bool _stableValue = false;
            bool _haveStable = false;
            int64_t _lastRawChangeUs = 0;
        };
    }
}
