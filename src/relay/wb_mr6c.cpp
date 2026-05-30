#include <log/log.h>

#include "wb_mr6c.h"

bool EDCommon::Relay::WBMR6C::setStateInternal(bool enable)
{
    if (!_mr6c->setRelayChannelState(_channel, enable)) {
        LOGE("relay", "failed to change relay state");
        return false;
    }

    return true;
}

std::pair<bool, bool> EDCommon::Relay::WBMR6C::isEnabledInternal()
{
    return _mr6c->getRelayChannelState(_channel);
}
