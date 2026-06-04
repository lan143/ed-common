#include <Arduino.h>
#include <log/log.h>
#include "command_consumer.h"
#include "command.h"

void EDCommon::Light::MQTTCommandConsumer::consume(std::string payload)
{
    LOGD("mqtt_command_consumer", "handle");

    if (payload == "on") {
        _light->setState(true);
    } else if (payload == "off") {
        _light->setState(false);
    } else {
        MQTTCommand command;
        if (!command.unmarshalJSON(payload.c_str())) {
            LOGE("mqtt_command_consumer", "cant unmarshal command");
            return;
        }

        if (_light->hasBrightnessControl() && command.getBrightness().second) {
            _light->setBrightness(command.getBrightness().first);
        }

        if (_light->hasTemperatureControl() && command.getTempColor().second) {
            _light->setTemperature(command.getTempColor().first);
        }

        if (_light->hasColorControl() && command.getColor().second) {
            _light->setColor(command.getColor().first);
        }
    }
}
