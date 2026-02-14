#ifndef SIMPLEBUTTON_H
#define SIMPLEBUTTON_H

#include <Arduino.h>

class SimpleButton
{
public:
    SimpleButton(uint8_t pin, uint32_t debounceDelay = 20);

    void begin();
    void update();
    bool wasPressed();
    bool isPressed();

private:
    uint8_t _pin;

    uint8_t _state;
    uint8_t _lastReading;

    uint32_t _lastDebounceTime;
    uint32_t _debounceDelay;

    bool _pressedEvent;
};

#endif
