#pragma once

#include <Arduino.h>
#include <PCF8574.h>

#include "./relay.h"

namespace EDCommon
{
    namespace Relay
    {
        class PCF8574P : public Relay
        {
        public:
            PCF8574P(PCF8574* driver) : _driver(driver) {}

            bool init(uint8_t channel, std::initializer_list<RelayOption> options)
            {
                _channel = channel;

                return Relay::init(options);
            }

        private:
            bool setStateInternal(bool enable);
            std::pair<bool, bool> isEnabledInternal();

        private:
            uint8_t _channel;

        private:
            PCF8574* _driver = nullptr;
        };
    }
}
