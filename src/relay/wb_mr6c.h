#pragma once

#include <Arduino.h>
#include <device/wb_mr6c.h>
#include <discovery.h>
#include <mqtt.h>

#include "relay.h"
#include "mqtt/state.h"
#include "mqtt/state_producer.h"

namespace EDCommon
{
    namespace Relay
    {
        class WBMR6C : public Relay
        {
        public:
            WBMR6C(EDWB::MR6C* mr6c) : Relay(), _mr6c(mr6c) { }

            bool init(uint8_t channel, std::initializer_list<RelayOption> options)
            {
                _channel = channel;

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
