#include <log/log.h>

#include "wb_mai6_pressure.h"

bool EDCommon::Sensor::WBMAI6Pressure::setupChannel()
{
    auto sensorType = _device->getSensorTypeN(_channel);
    if (!sensorType.second) {
        LOGE("setupChannel", "failed to get sensor type");
        return false;
    }

    if (sensorType.first != _sensorType) {
        LOGD("setupChannel", "change sensor type");

        if (!_device->setSensorTypeN(_channel, _sensorType)) {
            LOGE("setupChannel", "failed to set sensor type");
            return false;
        }
    }

    auto minCalculatedValue = _device->getMinCalculatedValueN(_channel);
    if (!minCalculatedValue.second) {
        LOGE("setupChannel", "failed to get min calculated value");
        return false;
    }

    if (minCalculatedValue.first != _minCalculatedValue) {
        LOGD("setupChannel", "change min calculated value");

        if (!_device->setMinCalculatedValueN(_channel, _minCalculatedValue)) {
            LOGE("setupChannel", "failed to set min calculated value");
            return false;
        }
    }

    auto maxCalculatedValue = _device->getMaxCalculatedValueN(_channel);
    if (!maxCalculatedValue.second) {
        LOGE("setupChannel", "failed to get max calculated value");
        return false;
    }

    if (maxCalculatedValue.first != _maxCalculatedValue) {
        LOGD("setupChannel", "change max calculated value");

        if (!_device->setMaxCalculatedValueN(_channel, _maxCalculatedValue)) {
            LOGE("setupChannel", "failed to set max calculated value");
            return false;
        }
    }

    LOGI("setupChannel", "setup complete");

    return true;
}

std::pair<float_t, bool> EDCommon::Sensor::WBMAI6Pressure::getValueInternal()
{
    auto value = _device->getCalculatedValueN(_channel);
    if (!value.second) {
        return std::make_pair(0.0f, false);
    }

    float_t physicalValue = ((float_t)value.first - (float_t)_minCalculatedValue) * _maxPhysicalValue
        / ((float_t)_maxCalculatedValue - (float_t)_minCalculatedValue);

    return std::make_pair(physicalValue, true);
}
