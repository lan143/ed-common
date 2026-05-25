#pragma once

#include <Arduino.h>

namespace EDCommon
{
    namespace Relay
    {
        class MQTTState
        {
        public:
            MQTTState() {}

            bool operator==(MQTTState& other);
            bool operator!=(MQTTState& other) { return !(*this == other); }

            std::string marshalJSON();

            void setState(bool enabled) { _enabled = {enabled, true}; }
        
        private:
            std::pair<bool, bool> _enabled;
        };
    }
}
