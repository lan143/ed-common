#include <algorithm>
#include <log/log.h>

#include "wb_mr6c.h"
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
            ->setStateOn("on")
            ->setStateOff("off");
    }

    auto enabledResult = isEnabled();
    setState(enabledResult.first);

    return true;
}

std::pair<bool, bool> EDCommon::Relay::WBMR6C::isEnabled()
{
    return _mr6c->getRelayChannelState(_config.channel);
}

bool EDCommon::Relay::WBMR6C::setState(bool enable)
{
    WBMR6CCommand cmd;
    cmd.enable = enable;
    if (xQueueSend(_commandQueue, &cmd, 0) != pdTRUE) {
        return false;
    }

    return true;
}

void EDCommon::Relay::WBMR6C::update()
{
    WBMR6CCommand cmd;
    while (xQueueReceive(_commandQueue, &cmd, 0) == pdTRUE) {
        setStateInternal(cmd.enable);
    }

    if ((_lastPublishStateTime + 60000000) < esp_timer_get_time()) {
        publishState();
    }

    if (_config.hasTimeout && (_lastTimeoutCheckTime + 1000000) < esp_timer_get_time()) {
        auto enabled = isEnabled();
        if (enabled.first && enabled.second && ((_lastEnabledTime + _config.timeout) < esp_timer_get_time())) {
            setState(false);
        }

        _lastTimeoutCheckTime = esp_timer_get_time();
    }
}

bool EDCommon::Relay::WBMR6C::publishState()
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

bool EDCommon::Relay::WBMR6C::setStateInternal(bool enable)
{
    if (!_mr6c->setRelayChannelState(_config.channel, enable)) {
        LOGE("relay", "failed to change relay state");
        return false;
    }

    _mqttState.setState(enable);
    publishState();

    _lastEnabledTime = esp_timer_get_time();

    return true;
}
