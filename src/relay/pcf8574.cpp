#include <log/log.h>

#include "./pcf8574.h"

bool EDCommon::Relay::PCF8574P::setStateInternal(bool enable)
{
    _driver->write(_channel, enable ? LOW : HIGH);

    return true;
}

std::pair<bool, bool> EDCommon::Relay::PCF8574P::isEnabledInternal()
{
    return {_driver->read(_channel) == LOW, true};
}
