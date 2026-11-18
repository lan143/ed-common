#include <cmath>

#include "./dht_sensor.h"

EDCommon::Sensor::DHTSensor::DHTSensor(uint8_t pin, uint8_t type) : _dht(pin, type)
{
}

bool EDCommon::Sensor::DHTSensor::begin()
{
    _dht.begin();

    return true;
}

std::pair<float_t, bool> EDCommon::Sensor::DHTSensor::getTemperature()
{
    float value = _dht.readTemperature();

    if (std::isnan(value)) {
        return {0.0f, false};
    }

    return {value, true};
}

std::pair<float_t, bool> EDCommon::Sensor::DHTSensor::getHumidity()
{
    // Within the DHT minimum sample window the Adafruit driver serves the
    // cached reading, so two sensors sharing one driver perform one bus read
    // per update cycle.
    float value = _dht.readHumidity();

    if (std::isnan(value)) {
        return {0.0f, false};
    }

    return {value, true};
}
