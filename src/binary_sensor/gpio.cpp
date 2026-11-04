#include "./gpio.h"

bool EDCommon::BinarySensor::GPIO::preInit()
{
    pinMode(_pin, INPUT);

    _config.deviceClass = _deviceClass;
    _config.reverse = _reverse;

    return true;
}

std::pair<bool, bool> EDCommon::BinarySensor::GPIO::isActiveInternal()
{
    uint8_t highCount = 0;

    for (uint8_t i = 0; i < _samples; i++) {
        if (digitalRead(_pin) == HIGH) {
            highCount++;
        }

        if ((i + 1) < _samples) {
            delayMicroseconds(_sampleDelayUs);
        }
    }

    return {highCount > (_samples / 2), true};
}
