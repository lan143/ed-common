#pragma once

#include <discovery.h>
#include <mqtt.h>

#include "light.h"
#include "mqtt/state_producer.h"
#include "mqtt/state.h"
#include "relay/relay.h"

namespace EDCommon
{
    namespace Light
    {
        struct RelayConfig
        {
            bool hasMQTTSupport = false;
            EDMQTT::MQTT* mqtt = nullptr;
            std::string topicPrefix;
            std::string controllerName;
            std::string name;
            std::string mqttCommandTopic;
            std::string mqttStateTopic;

            bool hasDiscovery = false;
            EDHA::DiscoveryMgr* discoveryMgr = nullptr;
            EDHA::Device* device = nullptr;
        };

        using RelayOption = std::function<void(RelayConfig&)>;

        inline RelayOption withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](RelayConfig& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;
            };
        }

        inline RelayOption withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](RelayConfig& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        class Relay : public Light
        {
        public:
            Relay(EDCommon::Relay::Relay* relay) : _relay(relay) {}
            bool init(std::initializer_list<RelayOption> options);

            bool setState(bool enable);
            std::pair<bool, bool> isEnabled();

            void update();

        private:
            bool publishState();

        private:
            RelayConfig _config;
            MQTTState _mqttState;
            int64_t _lastPublishStateTime = 0;

        private:
            EDCommon::Relay::Relay* _relay = nullptr;
            StateProducer* _stateProducer = nullptr;
        };
    }
}
