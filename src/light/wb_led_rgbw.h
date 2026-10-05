#pragma once

#include <device/wb_led.h>

#include "light.h"

namespace EDCommon
{
    namespace Light
    {
        class WBLedRGBW : public Light
        {
        public:
            WBLedRGBW(EDWB::LED* led) : Light(), _led(led) { }
            bool init(uint8_t switchChannel = 0, std::initializer_list<LightOption> options = {});

            std::pair<bool, bool> isEnabled() override;
            std::pair<uint8_t, bool> getBrightness() override;
            std::pair<CRGB, bool> getColor() override;

            bool hasBrightnessControl() const override { return true; };
            bool hasColorControl() const override { return true; };

        protected:
            bool setStateInternal(bool enable) override;
            bool setBrightnessInternal(uint8_t brightness) override;
            bool setColorInternal(CRGB color) override;

        private:
            uint8_t _switchChannel;
            uint8_t _brightness;

        private:
            EDWB::LED* _led = nullptr;
        };
    }
}
