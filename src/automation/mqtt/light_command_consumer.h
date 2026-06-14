#pragma once

#include <Arduino.h>
#include <consumer.h>

#include "../light.h"

namespace EDCommon
{
    namespace Automation
    {
        class LightCommandConsumer : public EDMQTT::Consumer
        {
        public:
            LightCommandConsumer(Light* light) : _light(light) { }

            void consume(std::string payload);

        private:
            Light* _light = nullptr;
        };
    }
}
