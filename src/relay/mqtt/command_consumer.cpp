#include <Arduino.h>
#include <log/log.h>
#include "command_consumer.h"

void EDCommon::Relay::MQTTCommandConsumer::consume(std::string payload)
{
    LOGD("mqtt_command_consumer", "handle relay command");

    bool enable;
    if (payload == "on") {
        enable = true;
    } else if (payload == "off") {
        enable = false;
    }

    if (!_relay->setState(enable)) {
        LOGE("mqtt_command_consumer", "failed to handle relay command");
    }
}
