#include <Arduino.h>
#include <log/log.h>
#include "command_consumer.h"

void EDCommon::Relay::MQTTCommandConsumer::consume(std::string payload)
{
    LOGD("mqtt_command_consumer", "handle");

    if (payload == "on") {
        _relay->setState(true);
    } else if (payload == "off") {
        _relay->setState(false);
    }
}
