#pragma once

#include <Arduino.h>
#include <device/wb_mr6c.h>
#include <discovery.h>
#include <mqtt.h>

#include "mqtt/state.h"
#include "mqtt/state_producer.h"

namespace EDCommon
{
    namespace Relay
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

            bool hasTimeout = false;
            int64_t timeout = 0;
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

        inline RelayOption withTimeout(int64_t timeout)
        {
            return [timeout](RelayConfig& c) {
                c.hasTimeout = true;
                c.timeout = timeout;
            };
        }

        class Relay
        {
        public:
            Relay()
            {
                _commandQueue = xQueueCreate(10, sizeof(bool));
            }

            bool init(std::initializer_list<RelayOption> options);

            bool setState(bool enable);
            std::pair<bool, bool> isEnabled();

            void update();

        protected:
            virtual bool setStateInternal(bool enable) = 0;
            virtual std::pair<bool, bool> isEnabledInternal() = 0;

        private:
            bool publishState();

        private:
            StateProducer* _stateProducer = nullptr;

        private:
            RelayConfig _config;
            MQTTState _mqttState;
            QueueHandle_t _commandQueue;
            int64_t _lastPublishStateTime = 0;
            int64_t _lastEnabledTime = 0;
            int64_t _lastTimeoutCheckTime = 0;
        };
    }
}
