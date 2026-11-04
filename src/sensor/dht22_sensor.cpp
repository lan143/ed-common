#include <cmath>

#include "./dht22_sensor.h"

EDCommon::Sensor::DHT22Sensor::DHT22Sensor(uint8_t pin) : _dht(pin, DHT22)
{
}

bool EDCommon::Sensor::DHT22Sensor::begin()
{
    _dht.begin();

    return true;
}

std::pair<float_t, bool> EDCommon::Sensor::DHT22Sensor::getTemperature()
{
    float value = _dht.readTemperature();

    if (std::isnan(value)) {
        return {0.0f, false};
    }

    return {value, true};
}

std::pair<float_t, bool> EDCommon::Sensor::DHT22Sensor::getHumidity()
{
    // Within the DHT22 minimum sample window the Adafruit driver serves the
    // cached reading, so two sensors sharing one driver perform one bus read
    // per update cycle.
    float value = _dht.readHumidity();

    if (std::isnan(value)) {
        return {0.0f, false};
    }

    return {value, true};
}
