#pragma once

#include <Arduino.h>
#include <FastLED.h>

namespace EDCommon
{
    namespace Light
    {
        class MQTTCommand
        {
        public:
            bool unmarshalJSON(const char* data);

            std::pair<uint8_t, bool> getBrightness() const { return _brightness; }
            std::pair<uint16_t, bool> getTempColor() const { return _tempColor; }
            std::pair<CRGB, bool> getColor() const { return _color; }

        private:
            std::pair<uint8_t, bool> _brightness;
            std::pair<uint16_t, bool> _tempColor;
            std::pair<CRGB, bool> _color;
        };
    }
}
