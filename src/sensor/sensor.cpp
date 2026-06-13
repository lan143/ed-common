#include <log/log.h>

#include "./sensor.h"

bool EDCommon::Sensor::Sensor::init(std::initializer_list<SensorOption> options)
{
    if (!preInit()) {
        LOGE("init", "failed to run pre init");
        return false;
    }

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
            ->setUnitOfMeasurement(_config.unitOfMeasurement)
            ->setDeviceClass(_config.deviceClass);
    }

    if (!postInit()) {
        LOGE("init", "failed to run post init");
        return false;
    }

    return true;
}

void EDCommon::Sensor::Sensor::update()
{
    if ((_lastUpdateTime + 10000000) < esp_timer_get_time()) {
        _currentValue = getValueInternal();
        if (_currentValue.second) {
            auto payload = EDUtils::formatString("%f", _currentValue.first);
            if (!_config.mqtt->publish(_config.mqttStateTopic.c_str(), payload.c_str(), true)) {
                LOGE("update", "failed to publish sensor update to mqtt");
            }
        } else {
            LOGE("update", "failed to get value from sensoe");
        }

        _lastUpdateTime = esp_timer_get_time();
    }
}
