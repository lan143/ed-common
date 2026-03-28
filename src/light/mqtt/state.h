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
        
        private:
            std::pair<bool, bool> _enabled;
            std::pair<uint8_t, bool> _brightness;
            std::pair<uint16_t, bool> _tempColor;
            std::pair<CRGB, bool> _color;
        };
    }
}