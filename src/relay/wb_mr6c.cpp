#include <algorithm>
#include <log/log.h>

#include "wb_mr6c.h"
#include "mqtt/state_producer.h"
#include "mqtt/command_consumer.h"

bool EDCommon::Relay::WBMR6C::init(uint8_t channel, std::initializer_list<WBMR6COption> options)
{
    _config.channel = channel;

    for (auto& opt : options) {
        opt(_config);
    }

    if (_config.hasMQTTSupport) {
        char mqttStateTopic[64] = {0};
        char mqttCommandTopic[64] = {0};
        std::string name = _config.name;

        std::replace(name.begin(), name.end(), ' ', '_');
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        snprintf(mqttStateTopic, 64, "%s/%s/state", _config.topicPrefix.c_str(), name.c_str());
        snprintf(mqttCommandTopic, 64, "%s/%s/set", _config.topicPrefix.c_str(), name.c_str());

        _config.mqttStateTopic = mqttStateTopic;
        _config.mqttCommandTopic = mqttCommandTopic;

        LOGD("Relay::init", "command topic: %s, state topic: %s", _config.mqttStateTopic.c_str(), _config.mqttCommandTopic.c_str());

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

        _config.discoveryMgr->addSwitch(
            _config.device,
            _config.name,
            discoveryObjectID,
            uniqueID
        )
            ->setCommandTopic(_config.mqttCommandTopic)
            ->setStateTopic(_config.mqttStateTopic)
            ->setValueTemplate("{{ value_json.enabled }}")
            ->setPayloadOn("on")
            ->setPayloadOff("off")
            ->setStateOn("true")
            ->setStateOff("false");
    }

    return true;
}

bool EDCommon::Relay::WBMR6C::setState(bool enable)
{
    if (!_mr6c->setRelayChannelState(_config.channel, enable)) {
        LOGE("relay", "failed to change relay state");
        return false;
    }

    if (_mqttStateMgr != nullptr) {
        _mqttStateMgr->getState().setState(enable);
    }

    _lastEnabledTime = esp_timer_get_time();

    return true;
}

std::pair<bool, bool> EDCommon::Relay::WBMR6C::isEnabled()
{
    return _mr6c->getRelayChannelState(_config.channel);
}

void EDCommon::Relay::WBMR6C::update()
{
    if (_mqttStateMgr != nullptr) {
        _mqttStateMgr->loop();
    }

    if (_config.hasTimeout && (_lastTimeoutCheckTime + 1000000) < esp_timer_get_time()) {
        auto enabled = isEnabled();
        if (enabled.first && enabled.second && ((_lastEnabledTime + _config.timeout) < esp_timer_get_time())) {
            setState(false);
        }

        _lastTimeoutCheckTime = esp_timer_get_time();
    }
}
