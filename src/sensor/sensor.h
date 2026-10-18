#pragma once

#include <Arduino.h>
#include <discovery.h>
#include <GyverFilters.h>
#include <mqtt.h>

namespace EDCommon
{
    namespace Sensor
    {
        struct SensorConfig
        {
            bool hasMQTTSupport = false;
            EDMQTT::MQTT* mqtt = nullptr;
            std::string topicPrefix;
            std::string controllerName;
            std::string name;
            std::string mqttStateTopic;

            bool hasDiscovery = false;
            EDHA::DiscoveryMgr* discoveryMgr = nullptr;
            EDHA::Device* device = nullptr;
            std::string unitOfMeasurement;
            std::string deviceClass;

            GKalman* filter = nullptr;

            bool binaryMode = false;
        };

        using SensorOption = std::function<void(SensorConfig&)>;

        inline SensorOption withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](SensorConfig& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;

                std::string sanitized = name;
                std::replace(sanitized.begin(), sanitized.end(), ' ', '_');
                std::transform(sanitized.begin(), sanitized.end(), sanitized.begin(), [](unsigned char ch) {
                    return std::tolower(ch);
                });

                c.mqttStateTopic = topicPrefix + "/" + sanitized + "/state";
            };
        }

        inline SensorOption withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](SensorConfig& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        inline SensorOption withBinaryMode()
        {
            return [](SensorConfig& c) {
                c.binaryMode = true;
            };
        }

        inline SensorOption withKalmanFilter(float meaEstimate, float q)
        {
            return [meaEstimate, q](SensorConfig& c) {
                c.filter = new GKalman(meaEstimate, q);
            };
        }

        class Sensor
        {
        public:
            bool init(uint8_t precision, int64_t updateInterval, std::initializer_list<SensorOption> options);
            void update();
            void setValue(float_t value);
            std::pair<float_t, bool> getValue() { return _currentValue; }

        protected:
            virtual bool preInit() { return true; }
            virtual bool postInit() { return true; }
            virtual std::pair<float_t, bool> getValueInternal() { return {0.0f, false}; }

        protected:
            SensorConfig _config;

        private:
            void publishState();

        private:
            std::pair<float_t, bool> _currentValue;
            std::pair<float_t, bool> _lastPublishedValue = {0.0f, false};
            int64_t _lastUpdateTime = 0;
            float_t _precision = 0.0f;
            int64_t _updateInterval = 0;
            int64_t _lastPublishStateTime = 0;
        };
    }
}
