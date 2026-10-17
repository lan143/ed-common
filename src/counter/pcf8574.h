#pragma once

#include <Arduino.h>
#include <PCF8574.h>

#include "./counter.h"

namespace EDCommon
{
    namespace Counter
    {
        class PCF8574P : public Counter
        {
        public:
            PCF8574P(PCF8574* driver) : _driver(driver) {}

            bool init(uint8_t pin, std::initializer_list<CounterOption> options)
            {
                _pin = pin;

                return Counter::init(options);
            }

        protected:
            int readPin() override;

        private:
            uint8_t _pin = 0;

        private:
            PCF8574* _driver = nullptr;
        };
    }
}
