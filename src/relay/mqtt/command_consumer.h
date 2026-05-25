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

            void consume(std::string payload);

        private:
            Relay* _relay = nullptr;
        };
    }
}
