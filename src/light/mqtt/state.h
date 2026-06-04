#pragma once

#include <Arduino.h>
#include <FastLED.h>

namespace EDCommon
{
    namespace Light
    {
        class MQTTState
        {
        public:
            MQTTState() {}

            bool operator==(MQTTState& other);
            bool operator!=(MQTTState& other) { return !(*this == other); }

            std::string marshalJSON();

            void setEnabled(bool enabled) { _enabled = {enabled, true}; }
            void setBrightness(uint8_t brightness) { _brightness = {brightness, true}; }
            void setTemperature(uint16_t tempColor) { _tempColor = {tempColor, true}; }
            void setColor(CRGB color) { _color = {color, true}; }

        private:
            std::pair<bool, bool> _enabled;
            std::pair<uint8_t, bool> _brightness;
            std::pair<uint16_t, bool> _tempColor;
            std::pair<CRGB, bool> _color;
        };
    }
}