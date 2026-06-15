#include <Arduino.h>
#include <log/log.h>

#include "./light_command_consumer.h"

void EDCommon::Automation::LightCommandConsumer::consume(std::string payload)
{
    LOGD("mqtt_command_consumer", "handle light automation command");

    bool enable = false;
    if (payload == "true") {
        enable = true;
    } else if (payload == "false") {
        enable = false;
    }

    if (!_light->changeNightModeState(enable)) {
        LOGE("mqtt_command_consumer", "failed to change light night mode");
    }
}
