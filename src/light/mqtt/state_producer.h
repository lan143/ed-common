#pragma once

#include <Arduino.h>
#include <mqtt.h>
#include <producer.h>
#include <state/state_producer.h>
#include "state.h"

namespace EDCommon
{
    namespace Light
    {
        class StateProducer : public EDMQTT::Producer, public EDUtils::StateProducer<MQTTState>
        {
        public:
            StateProducer(EDMQTT::MQTT* mqtt) : Producer(mqtt) {}

            bool publish(MQTTState* state);
        };
    }
}
