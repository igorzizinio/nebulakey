#include <Arduino.h>  // main Arduino library
#include <U8g2lib.h>  // display library
#include <Keyboard.h> // keyboard controlling library

////////////////////////////////////////////////////
// Previous and next buttons
////////////////////////////////////////////////////
constexpr const uint8_t PREVIOUS_BUTTON_PIN = 15;
constexpr const uint8_t NEXT_BUTTON_PIN = 4;

////////////////////////////////////////////////////
// Encoder
// TODO: refactor this into a separate class to make the code cleaner
////////////////////////////////////////////////////
constexpr const uint8_t CLK_PIN = 2;
constexpr const uint8_t DT_PIN = 3;
constexpr const uint8_t SW_PIN = 4;
constexpr const uint8_t ENABLED_LED_PIN = 16;
uint8_t lastEncoded = 0;
int8_t encoderValue = 0;
uint8_t prev_CLK_state;
uint8_t lastButtonState = HIGH;
uint8_t buttonState = HIGH;
uint32_t lastDebounceTime = 0;
constexpr uint32_t debounceDelay = 20; // ms
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Metadata (will be fetched from the host via software (Serial) and displayed on the OLED display)
String currentTrack = "No track playing";
////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  pinMode(PREVIOUS_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);

  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);

  Keyboard.begin();

  Serial.println("NebulaKey firmware started");

  prev_CLK_state = digitalRead(CLK_PIN);
}

void loop()
{

  uint8_t reading = digitalRead(SW_PIN);

  if (reading != lastButtonState)
  {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {

    if (reading != buttonState)
    {
      buttonState = reading;

      if (buttonState == LOW)
      {
        Serial.println("Encoder button pressed (play/pause)");
        Keyboard.consumerPress(KEY_PLAY_PAUSE);
        Keyboard.consumerRelease();
      }
    }
  }

  lastButtonState = reading;

  int MSB = digitalRead(CLK_PIN);
  int LSB = digitalRead(DT_PIN);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderValue++;

  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderValue--;

  if (encoderValue >= 4)
  {
    Serial.println("Volume +");
    Keyboard.consumerPress(KEY_VOLUME_INCREMENT);
    Keyboard.consumerRelease();
    encoderValue = 0;
  }

  if (encoderValue <= -4)
  {
    Serial.println("Volume -");
    Keyboard.consumerPress(KEY_VOLUME_DECREMENT);
    Keyboard.consumerRelease();
    encoderValue = 0;
  }

  lastEncoded = encoded;

  if (digitalRead(PREVIOUS_BUTTON_PIN) == LOW)
  {
    Serial.println("Previous button pressed");
    Keyboard.consumerPress(KEY_SCAN_PREVIOUS);
    Keyboard.consumerRelease();
  }

  if (digitalRead(NEXT_BUTTON_PIN) == LOW)
  {
    Serial.println("Next button pressed");
    Keyboard.consumerPress(KEY_SCAN_NEXT);
    Keyboard.consumerRelease();
  }

  while (Serial.available())
  {
    String line = Serial.readStringUntil('\n');
    if (line.startsWith("TRACK: "))
    {
      currentTrack = line.substring(7);
      Serial.print("Current track updated: ");
      Serial.println(currentTrack);
    }
  }
}
