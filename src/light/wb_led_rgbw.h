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
            WBLedRGBW(EDWB::LED* led, uint16_t nativeCCT = 4000, float whiteStrength = 1.0f) : Light(), _nativeCCT(nativeCCT), _whiteStrength(whiteStrength), _led(led) { }
            bool init(uint8_t switchChannel = 0, std::initializer_list<LightOption> options = {});

            std::pair<bool, bool> isEnabled() override;
            std::pair<uint8_t, bool> getBrightness() override;
            std::pair<CRGB, bool> getColor() override;
            std::pair<uint16_t, bool> getTemperature() override;

            bool hasBrightnessControl() const override { return true; };
            bool hasColorControl() const override { return true; };
            bool hasTemperatureControl() const override { return true; };

        protected:
            bool setStateInternal(bool enable) override;
            bool setBrightnessInternal(uint8_t brightness) override;
            bool setColorInternal(CRGB color) override;
            bool setTemperatureInternal(uint16_t temperature) override;

        private:
            uint8_t _switchChannel;
            uint8_t _brightness = 100;
            CRGB _lastColor = CRGB(255, 255, 255);
            uint16_t _nativeCCT;
            float _whiteStrength;
            uint16_t _temperature = 0;
            bool applyOutput();

            static CRGB kelvinToRGB(uint16_t kelvin);

        private:
            EDWB::LED* _led = nullptr;
        };
    }
}
