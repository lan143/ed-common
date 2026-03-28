#pragma once

#include <Arduino.h>
#include <FastLED.h>

namespace EDCommon
{
    namespace Light
    {
        class Light
        {
        public:
            virtual bool setState(bool enable) = 0;
            virtual std::pair<bool, bool> isEnabled() = 0;
        };

        class Dimmer
        {
        public:
            virtual bool setBrightness(uint8_t brightness) = 0;
            virtual std::pair<uint8_t, bool> getBrightness() = 0;
        };

        class ColorTemperatureSetter
        {
        public:
            virtual bool setTemperature(uint16_t temperature) = 0;
            virtual std::pair<uint16_t, bool> getTemperature() = 0;
        };

        class ColorSetter
        {
        public:
            virtual bool setColor(CRGB color) = 0;
            virtual std::pair<CRGB, bool> getColor() = 0;
        };
    }
}
