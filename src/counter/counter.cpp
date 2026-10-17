#include <algorithm>
#include <cctype>
#include <cmath>
#include <ExtStrings.h>
#include <Utils.h>
#include <log/log.h>

#include "./counter.h"

bool EDCommon::Counter::Counter::init(std::initializer_list<CounterOption> options)
{
    for (auto& opt : options) {
        opt(_config);
    }

    if (_config.persistence != nullptr) {
        _config.persistence->load(_currentValue, _lock);
    }

    if (_config.hasMQTTSupport) {
        LOGD("Counter::init", "state topic: %s", _config.mqttStateTopic.c_str());
    }

    if (_config.hasDiscovery && _config.hasMQTTSupport) {
        std::string objectID = _config.objectID;

        if (objectID.empty()) {
            objectID = _config.name;
        }

        std::replace(objectID.begin(), objectID.end(), ' ', '_');
        std::transform(objectID.begin(), objectID.end(), objectID.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        std::string controllerName = _config.controllerName;

        std::replace(controllerName.begin(), controllerName.end(), ' ', '_');
        std::transform(controllerName.begin(), controllerName.end(), controllerName.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        std::string uniqueID = EDUtils::formatString("%s_%s_%s", objectID.c_str(), controllerName.c_str(), EDUtils::getChipID());

        _config.discoveryMgr->addSensor(
            _config.device,
            _config.name,
            objectID,
            uniqueID
        )
            ->setStateTopic(_config.mqttStateTopic)
            ->setValueTemplate("{{ value }}")
            ->setUnitOfMeasurement(_config.unitOfMeasurement)
            ->setSensorStateClass(EDHA::SENSOR_STATE_CLASS_TOTAL)
            ->setDeviceClass(_config.deviceClass);
    }

    _dirty = true;

    return true;
}

void EDCommon::Counter::Counter::update()
{
    if ((_lastCheckTime + 250000) < esp_timer_get_time()) {
        int pinValue = readPin();

        if (pinValue == LOW && !_lock) {
            if (!_ponentialLockUnlock) {
                _ponentialLockUnlock = true;
            } else {
                _ponentialLockUnlock = false;
                _lock = true;
                _currentValue++;

                if (_config.persistence != nullptr) {
                    _config.persistence->save(_currentValue, true);
                }

                _lastFlowOfWaterActive = true;
                _lastFlowOfWaterActiveTime = esp_timer_get_time();
                _dirty = true;
            }
        } else if (pinValue == HIGH && _lock) {
            if (!_ponentialLockUnlock) {
                _ponentialLockUnlock = true;
            } else {
                _ponentialLockUnlock = false;
                _lock = false;

                if (_config.persistence != nullptr) {
                    _config.persistence->save(_currentValue, false);
                }
            }
        }

        _lastCheckTime = esp_timer_get_time();
    }

    if (_lastFlowOfWaterActive && (_lastFlowOfWaterActiveTime + 120000000) < esp_timer_get_time()) {
        _lastFlowOfWaterActive = false;
    }

    if (_config.hasMQTTSupport && _dirty) {
        publishState();
    }
}

void EDCommon::Counter::Counter::publishState()
{
    if (_config.mqtt == nullptr) {
        return;
    }

    auto payload = EDUtils::formatString("%.3f", (double)((float_t)_currentValue * (float_t)_config.cost / 1000.0f));

    if (_config.mqtt->publish(_config.mqttStateTopic.c_str(), payload.c_str(), true)) {
        _dirty = false;
    }
}

void EDCommon::Counter::Counter::setInitialValue(float_t value)
{
    _currentValue = (uint32_t)((value * 1000.0f) / (float_t)_config.cost);

    if (_config.persistence != nullptr) {
        _config.persistence->clear();
        _config.persistence->save(_currentValue, _lock);
    }

    _dirty = true;
}
