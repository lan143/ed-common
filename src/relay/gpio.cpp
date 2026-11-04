#include "./gpio.h"

bool EDCommon::Relay::GPIO::setStateInternal(bool enable)
{
    digitalWrite(_pin, (enable == _activeHigh) ? HIGH : LOW);
    _enabled = enable;

    return true;
}

std::pair<bool, bool> EDCommon::Relay::GPIO::isEnabledInternal()
{
    return {_enabled, true};
}
