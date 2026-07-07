#include "Uncoder.h"

Uncoder::Uncoder(uint8_t dtPin, uint8_t clkPin)
{
  this->dtPin = dtPin;
  this->clkPin = clkPin;
}

void Uncoder::begin()
{
  pinMode(dtPin, INPUT_PULLUP);
  pinMode(clkPin, INPUT_PULLUP);

  int MSB = digitalRead(clkPin);
  int LSB = digitalRead(dtPin);
  lastEncoded = (MSB << 1) | LSB;
}

void Uncoder::update()
{
  int MSB = digitalRead(clkPin);
  int LSB = digitalRead(dtPin);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderValue++;

  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderValue--;

  if (encoderValue >= 4)
  {
    direction = Direction::PLUS;
    encoderValue = 0;
  }
  else if (encoderValue <= -4)
  {
    direction = Direction::MINUS;
    encoderValue = 0;
  }
  else
  {
    direction = Direction::NEUTRAL;
  }

  lastEncoded = encoded;
}

Direction Uncoder::getDirection()
{
  return direction;
}
