#include "wb_mr6c.h"

bool EDCommon::BinarySensor::WBMR6C::init(uint8_t channel, std::initializer_list<WBMR6COption> options)
{
    _config.channel = channel;

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

void EDCommon::BinarySensor::WBMR6C::update()
{
    if ((_lastUpdateTime + 200000) < esp_timer_get_time()) {
        auto result = _mr6c->getInputChannelState(_config.channel);

        if (result.second && _contact != result) {
            if (!_config.mqtt->publish(_config.mqttStateTopic.c_str(), result.first ? "false" : "true", true)) {
                LOGE("update", "failed to publish update binary state");
                _lastUpdateTime = esp_timer_get_time();
                return;
            }

            _contact = result;
        } else if (!result.second) {
            LOGE("update", "failed to get MR6C input channel state");
        }

        _lastUpdateTime = esp_timer_get_time();
    }
}
