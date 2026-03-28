#include <ExtStrings.h>
#include <Json.h>
#include <Utils.h>

#include "state.h"

bool EDCommon::Light::MQTTState::operator==(MQTTState& other)
{
    return _enabled == other._enabled
        && _brightness == other._brightness
        && _tempColor == other._tempColor
        && _color == other._color;
}

std::string EDCommon::Light::MQTTState::marshalJSON()
{
    std::string payload = EDUtils::buildJson([this](JsonObject entity) {
        if (_enabled.second) {
            entity[F("enabled")] = _enabled.first ? "on" : "off";
        }

        if (_brightness.second) {
            entity[F("brightness")] = _brightness.first;
        }

        if (_tempColor.second) {
            entity[F("tempColor")] = _tempColor.first;
        }

        if (_color.second) {
            entity[F("color")] = EDUtils::formatString("%d,%d,%d", _color.first.r, _color.first.g, _color.first.b);
        }
    });

    return payload;
}
