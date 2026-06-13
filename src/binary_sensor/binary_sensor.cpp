#include "./binary_sensor.h"


bool EDCommon::BinarySensor::BinarySensor::init(int64_t updateInterval, std::initializer_list<Option> options)
{
    if (!preInit()) {
        LOGE("init", "failed to run pre init");
        return false;
    }

    _updateInterval = updateInterval * 1000;

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

        _config.discoveryMgr->addBinarySensor(
            _config.device,
            _config.name,
            discoveryObjectID,
            uniqueID
        )
            ->setStateTopic(_config.mqttStateTopic)
            ->setValueTemplate("{{ value }}")
            ->setPayloadOn("true")
            ->setPayloadOff("false")
            ->setDeviceClass(_config.deviceClass);
    }

    return true;
}

void EDCommon::BinarySensor::BinarySensor::update()
{
    if ((_lastUpdateTime + _updateInterval) < esp_timer_get_time()) {
        _isActive = isActiveInternal();
        if (_isActive.second) {
            bool publishResult = _config.reverse ? _config.mqtt->publish(_config.mqttStateTopic.c_str(), _isActive.first ? "false" : "true", true) : _config.mqtt->publish(_config.mqttStateTopic.c_str(), _isActive.first ? "true" : "false", true);
            if (!publishResult) {
                LOGE("update", "failed to publish update binary state");
                _lastUpdateTime = esp_timer_get_time();
                return;
            }
        } else {
            LOGE("update", "failed to get value from sensor");
        }

        _lastUpdateTime = esp_timer_get_time();
    }
}
