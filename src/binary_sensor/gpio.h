#pragma once

#include <Arduino.h>

#include "./binary_sensor.h"

namespace EDCommon
{
    namespace BinarySensor
    {
        class GPIO : public BinarySensor
        {
        public:
            GPIO(uint8_t pin, EDHA::DeviceClass* deviceClass, bool reverse, uint8_t samples = 1, uint16_t sampleDelayUs = 20)
                : _pin(pin), _deviceClass(deviceClass), _reverse(reverse), _samples(samples), _sampleDelayUs(sampleDelayUs) {}

        protected:
            bool preInit() override;
            std::pair<bool, bool> isActiveInternal() override;

        private:
            uint8_t _pin;
            EDHA::DeviceClass* _deviceClass = nullptr;
            bool _reverse = false;
            uint8_t _samples = 1;
            uint16_t _sampleDelayUs = 20;
        };
    }
}
