#include <log/log.h>

#include "water_level.h"

bool EDCommon::Sensor::WaterLevel::init(std::initializer_list<WaterLevelOption> options)
{
    for (auto& opt : options) {
        opt(_config);
    }

    if (_config.hasMQTTSupport) {
        char mqttStateTopic[64] = {0};
        std::string name = _config.name;

        std::replace(name.begin(), name.end(), ' ', '_');
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        snprintf(mqttStateTopic, 64, "%s/%s/state", _config.topicPrefix.c_str(), name.c_str());

        _config.mqttStateTopic = mqttStateTopic;

        LOGD("WaterLevel::init", "state topic: %s", _config.mqttStateTopic.c_str());
    }

    if (_config.hasDiscovery && _config.hasMQTTSupport) {
        std::string discoveryObjectID = _config.name;
        std::string controllerName = _config.controllerName;

        std::replace(controllerName.begin(), controllerName.end(), ' ', '_');
        std::transform(controllerName.begin(), controllerName.end(), controllerName.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        std::replace(discoveryObjectID.begin(), discoveryObjectID.end(), ' ', '_');
        std::transform(discoveryObjectID.begin(), discoveryObjectID.end(), discoveryObjectID.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        std::string uniqueID = EDUtils::formatString("%s_%s_%s", discoveryObjectID.c_str(), controllerName.c_str(), EDUtils::getChipID());

        _config.discoveryMgr->addSensor(
            _config.device,
            _config.name,
            discoveryObjectID,
            uniqueID
        )
            ->setStateTopic(_config.mqttStateTopic)
            ->setValueTemplate("{{ value }}")
            ->setUnitOfMeasurement("m");
    }

    _constantsLoaded = loadConstants();

    return true;
}

void EDCommon::Sensor::WaterLevel::update()
{
    if ((_lastUpdateTime + 10000000) < esp_timer_get_time()) {
        _currentValue = getWaterLevel();
        if (_currentValue.second) {
            auto payload = EDUtils::formatString("%f", _currentValue.first);
            if (!_config.mqtt->publish(_config.mqttStateTopic.c_str(), payload.c_str(), true)) {
                LOGE("update", "failed to publish water level update to mqtt");
            }
        } else {
            LOGE("update", "failed to update current water level");
        }

        _lastUpdateTime = esp_timer_get_time();
    }
}

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

std::pair<float_t, bool> EDCommon::Sensor::WaterLevel::getWaterLevel()
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
