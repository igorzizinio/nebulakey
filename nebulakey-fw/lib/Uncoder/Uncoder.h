#ifndef UNCODER_H
#define UNCODER_H

#include <Arduino.h>

class Uncoder
{
public:
  Uncoder(uint8_t dtPin, uint8_t clkPin);

  void begin();
  void update();
  int8_t getDirection(); // -1, 0 ou 1

private:
  uint8_t dtPin;
  uint8_t clkPin;

  int lastEncoded = 0;
  int encoderValue = 0;
  int8_t direction = 0;
};

#endif
