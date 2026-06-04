#pragma once

#include <discovery.h>
#include <mqtt.h>

#include "light.h"
#include "mqtt/state_producer.h"
#include "mqtt/state.h"
#include "relay/relay.h"

namespace EDCommon
{
    namespace Light
    {
        class Relay : public Light
        {
        public:
            Relay(EDCommon::Relay::Relay* relay) : _relay(relay) { }

            std::pair<bool, bool> isEnabled() override { return _relay->isEnabled(); }

            void update()
            {
                _relay->update();
                Light::update();
            }

        protected:
            bool setStateInternal(bool enable) override { return _relay->setState(enable); }

        private:
            EDCommon::Relay::Relay* _relay = nullptr;
        };
    }
}
