#pragma once

#include <Arduino.h>

namespace EDCommon
{
    namespace Relay
    {
        class Relay
        {
        public:
            virtual bool setState(bool enable) = 0;
            virtual std::pair<bool, bool> isEnabled() = 0;
            virtual void update() = 0;
        };
    }
}
