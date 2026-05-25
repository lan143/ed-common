#include "state_producer.h"

#include "state.h"

bool EDCommon::Relay::StateProducer::publish(EDCommon::Relay::MQTTState* state)
{
    if (!_mqtt->isConnected()) {
        return false;
    }

    std::string payload = state->marshalJSON();
    return _mqtt->publish(_topic, payload.c_str(), false);
}
