#include <algorithm>
#include <log/log.h>

#include "mqtt/command_consumer.h"
#include "light/light.h"

bool EDCommon::Light::Light::init(std::initializer_list<LightOption> options)
{
    for (auto& opt : options) {
        opt(_config);
    }

    if (_config.hasMQTTSupport) {
        char mqttStateTopic[64] = {0};
        char mqttCommandTopic[64] = {0};
        std::string controllerName = _config.controllerName;
        std::string name = _config.name;

        std::replace(name.begin(), name.end(), ' ', '_');
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        snprintf(mqttStateTopic, 64, "%s/%s/state", _config.topicPrefix.c_str(), name.c_str());
        snprintf(mqttCommandTopic, 64, "%s/%s/set", _config.topicPrefix.c_str(), name.c_str());

        _config.mqttStateTopic = mqttStateTopic;
        _config.mqttCommandTopic = mqttCommandTopic;

        LOGD("RelayLight::init", "command topic: %s, state topic: %s", _config.mqttStateTopic.c_str(), _config.mqttCommandTopic.c_str());

        _stateProducer = new StateProducer(_config.mqtt);
        _stateProducer->init(_config.mqttStateTopic.c_str());

        auto commandConsumer = new MQTTCommandConsumer(this);
        commandConsumer->init(_config.mqttCommandTopic.c_str());
        _config.mqtt->subscribe(commandConsumer);
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

        auto discovery = _config.discoveryMgr->addLight(
            _config.device,
            _config.name,
            discoveryObjectID,
            uniqueID
        )
            ->setCommandTopic(_config.mqttCommandTopic)
            ->setStateTopic(_config.mqttStateTopic)
            ->setStateValueTemplate("{{ value_json.enabled }}")
            ->setPayloadOn("on")
            ->setPayloadOff("off");

        if (hasBrightnessControl()) {
            discovery->setBrightnessCommandTopic(_config.mqttCommandTopic)
            ->setBrightnessCommandTemplate("{\"brightness\": {{ value }} }")
            ->setBrightnessStateTopic(_config.mqttStateTopic)
            ->setBrightnessValueTemplate("{{ value_json.brightness }}")
            ->setBrightnessScale(100)
            ->setColorTempKelvin(true)
            ->setColorTempCommandTemplate("{\"tempColor\": {{ value }} }")
            ->setColorTempCommandTopic(_config.mqttCommandTopic)
            ->setColorTempStateTopic(_config.mqttStateTopic)
            ->setColorTempValueTemplate("{{ value_json.tempColor }}")
            ->setMinKelvin(2700) // @todo: move to options
            ->setMaxKelvin(6000); // @todo: move to options
        }
    }

    auto enabledResult = isEnabled();
    setState(enabledResult.first);

    return true;
}

bool EDCommon::Light::Light::setState(bool enable)
{
    LightCommand cmd;
    cmd.enable = {enable, true};
    if (xQueueSend(_commandQueue, &cmd, 0) != pdTRUE) {
        return false;
    }

    return true;
}

bool EDCommon::Light::Light::setBrightness(uint8_t brightness)
{
    if (!hasBrightnessControl()) {
        return false;
    }

    LightCommand cmd;
    cmd.brightness = {brightness, true};
    if (xQueueSend(_commandQueue, &cmd, 0) != pdTRUE) {
        return false;
    }

    return true;
}

bool EDCommon::Light::Light::setTemperature(uint16_t temperature)
{
    if (!hasTemperatureControl()) {
        return false;
    }

    LightCommand cmd;
    cmd.tempColor = {temperature, true};
    if (xQueueSend(_commandQueue, &cmd, 0) != pdTRUE) {
        return false;
    }

    return true;
}

bool EDCommon::Light::Light::setColor(CRGB color)
{
    if (!hasColorControl()) {
        return false;
    }

    LightCommand cmd;
    cmd.color = {color, true};
    if (xQueueSend(_commandQueue, &cmd, 0) != pdTRUE) {
        return false;
    }

    return true;
}

void EDCommon::Light::Light::update()
{
    LightCommand cmd;
    while (xQueueReceive(_commandQueue, &cmd, 0) == pdTRUE) {
        if (cmd.enable.second) {
            if (setStateInternal(cmd.enable.first)) {
                _mqttState.setEnabled(cmd.enable.first);
                if (!publishState()) {
                    LOGE("update", "failed to publish state");
                }
            } else {
                LOGE("update", "failed to update light state");
            }
        }

        if (cmd.brightness.second) {
            if (setBrightnessInternal(cmd.brightness.first)) {
                _mqttState.setBrightness(cmd.brightness.first);
                if (!publishState()) {
                    LOGE("update", "failed to publish state");
                }
            } else {
                LOGE("update", "failed to update light brightness");
            }
        }

        if (cmd.tempColor.second) {
            if (setTemperatureInternal(cmd.tempColor.first)) {
                _mqttState.setTemperature(cmd.tempColor.first);
                if (!publishState()) {
                    LOGE("update", "failed to publish state");
                }
            } else {
                LOGE("update", "failed to update light temperature");
            }
        }

        if (cmd.color.second) {
            if (setColorInternal(cmd.color.first)) {
                _mqttState.setColor(cmd.color.first);
                if (!publishState()) {
                    LOGE("update", "failed to publish state");
                }
            } else {
                LOGE("update", "failed to update light color");
            }
        }
    }

    if ((_lastPublishStateTime + 60000000) < esp_timer_get_time()) {
        publishState();
    }
}

bool EDCommon::Light::Light::publishState()
{
    if (_stateProducer == nullptr) {
        return true;
    }

    auto result = _stateProducer->publish(&_mqttState);
    if (!result) {
        return false;
    }

    _lastPublishStateTime = esp_timer_get_time();

    return true;
}
