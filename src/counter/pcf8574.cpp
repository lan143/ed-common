#include "./pcf8574.h"

int EDCommon::Counter::PCF8574P::readPin()
{
    return _driver->read(_pin);
}
