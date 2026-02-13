#include <Arduino.h>  // main Arduino library
#include <U8g2lib.h>  // display library
#include <ezButton.h> // encoder button library

constexpr const uint8_t PREVIOUS_BUTTON_PIN = 20;
constexpr const uint8_t NEXT_BUTTON_PIN = 21;

// Encoder pins

// Placeholders, got from https://arduinogetstarted.com/tutorials/arduino-rotary-encoder
constexpr const uint8_t CLK_PIN = 2;
constexpr const uint8_t DT_PIN = 3;
constexpr const uint8_t SW_PIN = 4;

ezButton encoderButton(SW_PIN);

void setup()
{
  Serial.begin(115200);

  pinMode(PREVIOUS_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);

  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  encoderButton.setDebounceTime(50);
}

void loop()
{
  encoderButton.loop();

  if (encoderButton.isPressed())
  {
    Serial.println("Encoder button pressed (play/pause)");
  }
}
