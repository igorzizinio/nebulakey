#include "SimpleButton.h"

SimpleButton::SimpleButton(uint8_t pin, uint32_t debounceDelay)
{
    _pin = pin;
    _debounceDelay = debounceDelay;
}

void SimpleButton::begin()
{
    pinMode(_pin, INPUT_PULLUP);

    _state = HIGH;
    _lastReading = HIGH;
    _lastDebounceTime = 0;
    _pressedEvent = false;
}

void SimpleButton::update()
{
    uint8_t reading = digitalRead(_pin);

    if (reading != _lastReading)
    {
        _lastDebounceTime = millis();
    }

    if ((millis() - _lastDebounceTime) > _debounceDelay)
    {
        if (reading != _state)
        {
            _state = reading;

            if (_state == LOW) // pressionado (INPUT_PULLUP)
            {
                _pressedEvent = true;
            }
        }
    }

    _lastReading = reading;
}

bool SimpleButton::wasPressed()
{
    if (_pressedEvent)
    {
        _pressedEvent = false;
        return true;
    }
    return false;
}

bool SimpleButton::isPressed()
{
    return _state == LOW;
}
