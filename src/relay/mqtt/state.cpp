#include <ExtStrings.h>
#include <Json.h>
#include <Utils.h>

#include "state.h"

bool EDCommon::Relay::MQTTState::operator==(MQTTState& other)
{
    return _enabled == other._enabled;
}

std::string EDCommon::Relay::MQTTState::marshalJSON()
{
    std::string payload = EDUtils::buildJson([this](JsonObject entity) {
        if (_enabled.second) {
            entity[F("enabled")] = _enabled.first ? "on" : "off";
        }
    });

    return payload;
}
