#include <log/log.h>

#include "water_level.h"

bool EDCommon::Sensor::WaterLevel::loadConstants()
{
    auto unitOfMeasurement = _qdy30a->getUnitOfMeasurement();
    if (!unitOfMeasurement.second) {
        return false;
    }

    auto dotPosition = _qdy30a->getDotPosition();
    if (!dotPosition.second) {
        return false;
    }

    _unitOfMeasurement = unitOfMeasurement.first;
    _dotPosition = dotPosition.first;

    LOGD("loadConstants", "unit of measurement: %d, dot position: %d", _unitOfMeasurement, _dotPosition);

    return true;
}

std::pair<float_t, bool> EDCommon::Sensor::WaterLevel::getValueInternal()
{
    if (!_constantsLoaded) {
        _constantsLoaded = loadConstants();
    }

    if (!_constantsLoaded) {
        return std::make_pair(0.0f, false);
    }

    auto value = _qdy30a->getLevel();
    if (!value.second) {
        return value;
    }

    float_t convertLevel = (float_t)value.first;

    switch (_unitOfMeasurement) {
        case 16: // m
            break;
        case 17: // cm
            convertLevel /= 100.0f;
            break;
        case 18: // mm
            convertLevel /= 1000.0f;
            break;
        default:
            return std::make_pair(0.0f, false);
    }

    switch (_dotPosition) {
        case 1:
            convertLevel /= 10.0f;
            break;
        case 2:
            convertLevel /= 100.0f;
            break;
        case 3:
            convertLevel /= 1000.0f;
            break;
        default:
            return std::make_pair(0.0f, false);
    }

    return std::make_pair(std::round(convertLevel * 1000.0f) / 1000.0f, true);
}
