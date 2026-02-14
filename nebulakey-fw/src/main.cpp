#include <Arduino.h>  // main Arduino library
#include <U8g2lib.h>  // display library
#include <ezButton.h> // encoder button library
#include <Keyboard.h> // keyboard controlling library


constexpr const uint8_t PREVIOUS_BUTTON_PIN = 20;
constexpr const uint8_t NEXT_BUTTON_PIN = 21;

// Encoder pins

// Placeholders, got from https://arduinogetstarted.com/tutorials/arduino-rotary-encoder
constexpr const uint8_t CLK_PIN = 2;
constexpr const uint8_t DT_PIN = 3;
constexpr const uint8_t SW_PIN = 4;

constexpr const uint8_t ENABLED_LED_PIN = 16;

uint8_t prev_CLK_state;

ezButton encoderButton(SW_PIN);

void setup()
{
  Serial.begin(115200);
  pinMode(PREVIOUS_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);

  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  encoderButton.setDebounceTime(50);

  Keyboard.begin();
  

  Serial.println("NebulaKey firmware started");

   prev_CLK_state = digitalRead(CLK_PIN);
}

void loop()
{
  encoderButton.loop();

  if (encoderButton.isPressed())
  {
    Serial.println("Encoder button pressed (play/pause)");
    Keyboard.consumerPress(KEY_PLAY_PAUSE);
    Keyboard.consumerRelease();
  }

  uint8_t clkState = digitalRead(CLK_PIN);

  // If the state of CLK is changed, then pulse occurred
  if (clkState != prev_CLK_state && clkState == LOW) {
    // the encoder is rotating in counter-clockwise direction => decrease the counter
    if (digitalRead(DT_PIN) == LOW) {
      Serial.println("Encoder turned counter-clockwise (vol down)");
      Keyboard.consumerPress(KEY_VOLUME_DECREMENT);
      Keyboard.consumerRelease();
    } else {
      Serial.println("Encoder turned clockwise (vol up)");
      Keyboard.consumerPress(KEY_VOLUME_INCREMENT);
      Keyboard.consumerRelease();
    }
  }
  prev_CLK_state = clkState;
}
