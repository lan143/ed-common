#include "state_producer.h"

#include "state.h"

bool EDCommon::Light::StateProducer::publish(EDCommon::Light::MQTTState* state)
{
    if (!_mqtt->isConnected()) {
        return false;
    }

    std::string payload = state->marshalJSON();
    return _mqtt->publish(_topic, payload.c_str(), false);
}
