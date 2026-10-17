#pragma once

#include <stdint.h>

namespace EDCommon
{
    namespace Counter
    {
        struct CounterPersistence
        {
            virtual ~CounterPersistence() = default;

            virtual void load(uint32_t& value, bool& lock) = 0;
            virtual void save(uint32_t value, bool lock) = 0;
            virtual void clear() = 0;
        };
    }
}
