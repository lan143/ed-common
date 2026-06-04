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
            MQTTCommandConsumer(Light* light) : _light(light) { }

            void consume(std::string payload);

        private:
            Light* _light = nullptr;
        };
    }
}
