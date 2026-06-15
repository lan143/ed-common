#pragma once

#include <Arduino.h>
#include <discovery.h>
#include <mqtt.h>

#include "../sensor/sensor.h"

namespace EDCommon
{
    namespace Automation
    {
        struct Config
        {
            bool hasMQTTSupport = false;
            EDMQTT::MQTT* mqtt = nullptr;
            std::string topicPrefix;
            std::string controllerName;
            std::string name;
            std::string mqttStateTopic;
            std::string mqttCommandTopic;

            bool hasDiscovery = false;
            EDHA::DiscoveryMgr* discoveryMgr = nullptr;
            EDHA::Device* device = nullptr;
            EDHA::DeviceClass* deviceClass = nullptr;

            EDCommon::Sensor::Sensor* lightLevel = nullptr;
            float_t lowLightLevel = 0;

            bool hasNightMode = false;
        };

        using Option = std::function<void(Config&)>;

        inline Option withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](Config& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;
            };
        }

        inline Option withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](Config& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        inline Option withLightLevelSensor(EDCommon::Sensor::Sensor* lightLevel, float_t lowLightLevel)
        {
            return [lightLevel, lowLightLevel](Config& c) {
                c.lightLevel = lightLevel;
                c.lowLightLevel = lowLightLevel;
            };
        }

        inline Option withNightMode()
        {
            return [](Config& c) {
                c.hasNightMode = true;
            };
        }
    }
}