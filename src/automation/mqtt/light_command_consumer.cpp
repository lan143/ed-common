#include <Arduino.h>
#include <log/log.h>

#include "./light_command_consumer.h"

void EDCommon::Automation::LightCommandConsumer::consume(std::string payload)
{
    LOGD("mqtt_command_consumer", "handle light automation command");

    bool enable;
    if (payload == "true") {
        enable = true;
    } else if (payload == "false") {
        enable = false;
    }

    _light->changeNightModeState(enable);
}
