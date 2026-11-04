#pragma once

#include <Arduino.h>

#include "./relay.h"

namespace EDCommon
{
    namespace Relay
    {
        class GPIO : public Relay
        {
        public:
            bool init(uint8_t pin, bool activeHigh, std::initializer_list<RelayOption> options)
            {
                _pin = pin;
                _activeHigh = activeHigh;

                pinMode(_pin, OUTPUT);
                digitalWrite(_pin, _activeHigh ? LOW : HIGH);
                _enabled = false;

                return Relay::init(options);
            }

        private:
            bool setStateInternal(bool enable) override;
            std::pair<bool, bool> isEnabledInternal() override;

        private:
            uint8_t _pin = 0;
            bool _activeHigh = true;
            bool _enabled = false;
        };
    }
}
