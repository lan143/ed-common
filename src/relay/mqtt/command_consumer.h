#pragma once

#include <Arduino.h>
#include <consumer.h>

#include "relay/relay.h"

namespace EDCommon
{
    namespace Relay
    {
        class MQTTCommandConsumer : public EDMQTT::Consumer
        {
        public:
            MQTTCommandConsumer(Relay* relay) : _relay(relay) { }

            void consume(std::string payload) override;

            // Retained commands are HA-default-replayed on every (re)subscribe and would silently override newer persisted state.
            bool acceptRetained() const override { return false; }

        private:
            Relay* _relay = nullptr;
        };
    }
}
