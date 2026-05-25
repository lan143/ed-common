#include <algorithm>
#include <log/log.h>

#include "mqtt/state_producer.h"
#include "mqtt/command_consumer.h"
#include "relay_light.h"

bool EDCommon::Light::Relay::init(std::initializer_list<RelayOption> options)
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

        auto stateProducer = new StateProducer(_config.mqtt);
        stateProducer->init(_config.mqttStateTopic.c_str());
        _mqttStateMgr = new EDUtils::StateMgr<MQTTState>(stateProducer);

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

        _config.discoveryMgr->addLight(
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
    }

    return true;
}

bool EDCommon::Light::Relay::setState(bool enable)
{
    if (!_relay->setState(enable)) {
        LOGE("light", "failed to change light state");
        return false;
    }

    if (_mqttStateMgr != nullptr) {
        _mqttStateMgr->getState().setEnabled(enable);
    }

    return true;
}

std::pair<bool, bool> EDCommon::Light::Relay::isEnabled()
{
    return _relay->isEnabled();
}

void EDCommon::Light::Relay::update()
{
    _relay->update();

    if (_mqttStateMgr != nullptr) {
        _mqttStateMgr->loop();
    }
}
