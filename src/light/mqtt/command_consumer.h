#pragma once

#include <Arduino.h>
#include <consumer.h>

#include "light/light.h"

namespace EDCommon
{
    namespace Light
    {
        class MQTTCommandConsumer : public EDMQTT::Consumer
        {
        public:
            MQTTCommandConsumer(Light* light) : _light(light)
            {
                _dimmer = dynamic_cast<Dimmer*>(light);
                _colorTemperatureSetter = dynamic_cast<ColorTemperatureSetter*>(light);
                _colorSetter = dynamic_cast<ColorSetter*>(light);
            }

            void consume(std::string payload);

        private:
            Light* _light = nullptr;
            Dimmer* _dimmer = nullptr;
            ColorTemperatureSetter* _colorTemperatureSetter = nullptr;
            ColorSetter* _colorSetter = nullptr;
        };
    }
}
