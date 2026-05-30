#pragma once

#include <Arduino.h>
#include <device/wb_mr6c.h>
#include <discovery.h>
#include <mqtt.h>

#include "relay.h"
#include "mqtt/state.h"
#include "mqtt/state_producer.h"

namespace EDCommon
{
    namespace Relay
    {
        struct WBMR6CConfig
        {
            uint8_t channel = 0;

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

        struct WBMR6CCommand
        {
            bool enable;
        };

        using WBMR6COption = std::function<void(WBMR6CConfig&)>;

        inline WBMR6COption withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](WBMR6CConfig& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;
            };
        }

        inline WBMR6COption withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](WBMR6CConfig& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        inline WBMR6COption withTimeout(int64_t timeout)
        {
            return [timeout](WBMR6CConfig& c) {
                c.hasTimeout = true;
                c.timeout = timeout;
            };
        }

        class WBMR6C : public Relay
        {
        public:
            WBMR6C(EDWB::MR6C* mr6c) : _mr6c(mr6c)
            {
                _commandQueue = xQueueCreate(10, sizeof(WBMR6CCommand));
            }

            bool init(uint8_t channel, std::initializer_list<WBMR6COption> options);

            bool setState(bool enable);
            std::pair<bool, bool> isEnabled();

            void update();

        private:
            bool publishState();
            bool setStateInternal(bool enable);

        private:
            EDWB::MR6C* _mr6c = nullptr;
            StateProducer* _stateProducer = nullptr;

        private:
            WBMR6CConfig _config;
            MQTTState _mqttState;
            QueueHandle_t _commandQueue;
            int64_t _lastPublishStateTime = 0;
            int64_t _lastEnabledTime = 0;
            int64_t _lastTimeoutCheckTime = 0;
        };
    }
}
