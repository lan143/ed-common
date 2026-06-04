#pragma once

#include <device/wb_led.h>

#include "light.h"

namespace EDCommon
{
    namespace Light
    {
        class WBLedCCT : public Light
        {
        public:
            WBLedCCT(EDWB::LED* led) : _led(led) { }
            bool init(uint8_t cctChannel, std::initializer_list<LightOption> options, uint8_t switchChannel = 0);

            std::pair<bool, bool> isEnabled() override;
            std::pair<uint8_t, bool> getBrightness() override;
            std::pair<uint16_t, bool> getTemperature() override;

            bool hasBrightnessControl() const override { return true; };
            bool hasTemperatureControl() const override { return true; };

        protected:
            bool setStateInternal(bool enable) override;
            bool setBrightnessInternal(uint8_t brightness) override;
            bool setTemperatureInternal(uint16_t temperature) override;

        private:
            uint8_t _cctChannel;
            uint8_t _switchChannel;

        private:
            EDWB::LED* _led = nullptr;
        };
    }
}
