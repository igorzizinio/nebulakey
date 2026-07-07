#ifndef UNCODER_H
#define UNCODER_H

#include <Arduino.h>

enum class Direction {
  MINUS,
  NEUTRAL,
  PLUS
};

class Uncoder
{
public:
  Uncoder(uint8_t dtPin, uint8_t clkPin);

  void begin();
  void update();
  Direction getDirection(); // MINUS, NEUTRAL or PLUS

private:
  uint8_t dtPin;
  uint8_t clkPin;

  int lastEncoded = 0;
  int encoderValue = 0;
  Direction direction = Direction::NEUTRAL;
};

#endif
