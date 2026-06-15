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
        char mqttStateTopic[256] = {0};
        std::string name = _config.name;

        std::replace(name.begin(), name.end(), ' ', '_');
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        snprintf(mqttStateTopic, 256, "%s/%s/state", _config.topicPrefix.c_str(), name.c_str());

        _config.mqttStateTopic = mqttStateTopic;
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
        auto isActive = isActiveInternal();

        if (isActive.second) {
            if (_config.reverse) {
                isActive.first = !isActive.first;
            }

            if (isActive != _isActive) {
                _isActive = isActive;

                publishState();
            }
        } else {
            LOGE("update", "failed to get value from sensor");
        }

        _lastUpdateTime = esp_timer_get_time();
    }

    if ((_lastSendStateTime + 60000000) < esp_timer_get_time()) {
        publishState();
    }
}

void EDCommon::BinarySensor::BinarySensor::publishState()
{
    if (_config.mqtt == nullptr) {
        return;
    }

    if (!_isActive.second) {
        return;
    }

    if (!_config.mqtt->publish(_config.mqttStateTopic.c_str(), _isActive.first ? "true" : "false", true)) {
        LOGE("update", "failed to publish update binary state");
    } else {
        _lastSendStateTime = esp_timer_get_time();
    }
}
