#pragma once

#include <Arduino.h>
#include <device/wb_mr6c.h>

#include "relay.h"

namespace EDCommon
{
    namespace Relay
    {
        class WBMR6C : public Relay
        {
        public:
            WBMR6C(EDWB::MR6C* mr6c) : Relay(), _mr6c(mr6c) { }

            bool init(uint8_t channel, int8_t switchChannel, std::initializer_list<RelayOption> options)
            {
                _channel = channel;

                if (switchChannel >= 0 && !_mr6c->setInputMode(switchChannel, EDWB::MR6C_INPUT_MODE_BUTTON_WITHOUT_LOCKING)) {
                    LOGE("init", "failed to set input mode for channel %d", switchChannel);
                    return false;
                }

                return Relay::init(options);
            }

        private:
            bool setStateInternal(bool enable);
            std::pair<bool, bool> isEnabledInternal();

        private:
            EDWB::MR6C* _mr6c = nullptr;

        private:
            uint8_t _channel;
        };
    }
}
