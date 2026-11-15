#include <log/log.h>

#include "./pcf8574.h"

bool EDCommon::BinarySensor::PCF8574P::preInit()
{
    _config.deviceClass = _deviceClass;
    _config.reverse = _reverse;
    _config.channel = _channel;

    if (_driver == nullptr) {
        LOGE("init", "pcf8574 driver is not set");
        return false;
    }

    return true;
}

std::pair<bool, bool> EDCommon::BinarySensor::PCF8574P::isActiveInternal()
{
    // PCF8574::read() performs a fresh I2C read8() on every call but returns the
    // stale _dataIn on an I2C error (lastError() is not reset by a successful
    // read, so it cannot be used for per-read validity). Reads are therefore
    // treated as valid here; the debounce below absorbs single-sample glitches
    // and an I2C failure simply repeats the last raw value.
    uint8_t bit = _driver->read(_channel) ? 1 : 0;

    if (_debounceMs == 0) {
        return {bit == 1, true};
    }

    int64_t now = esp_timer_get_time();

    // First ever sample: accept the raw value as stable immediately so there
    // is no boot dead time.
    if (!_haveStable) {
        _haveStable = true;
        _lastRaw = bit;
        _stableValue = bit == 1;
        _lastRawChangeUs = now;

        return {_stableValue, true};
    }

    if (bit != _lastRaw) {
        // Raw value moved away from the last seen level: remember when it
        // changed and keep reporting the currently stable value until the new
        // level has been held for the full debounce interval.
        _lastRaw = bit;
        _lastRawChangeUs = now;

        return {_stableValue, true};
    }

    if ((_stableValue != (bit == 1)) && ((now - _lastRawChangeUs) >= ((int64_t)_debounceMs * 1000))) {
        _stableValue = bit == 1;
    }

    return {_stableValue, true};
}
