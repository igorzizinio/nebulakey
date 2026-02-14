#include <Arduino.h>  // main Arduino library
#include <U8g2lib.h>  // display library
#include <Keyboard.h> // keyboard controlling library
#include <SimpleButton.h>
#include <Bitmaps.h>

////////////////////////////////////////////////////
// PIN definitions
////////////////////////////////////////////////////
constexpr const uint8_t CLK_PIN = 2;
constexpr const uint8_t DT_PIN = 3;
constexpr const uint8_t SW_PIN = 1;
constexpr const uint8_t PREVIOUS_BUTTON_PIN = 15;
constexpr const uint8_t NEXT_BUTTON_PIN = 14;

constexpr const uint8_t DISPLAY_SDA = 4;
constexpr const uint8_t DISPLAY_SCL = 5;
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Encoder values
// TODO: refactor this into a separate class to make the code cleaner
////////////////////////////////////////////////////
uint8_t lastEncoded = 0;
int8_t encoderValue = 0;
uint8_t prev_CLK_state;
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Metadata (will be fetched from the host via software (Serial) and displayed on the OLED display)
String currentTrack = "No track playing";
////////////////////////////////////////////////////

SimpleButton playPauseButton(SW_PIN, 50);
SimpleButton prevButton(PREVIOUS_BUTTON_PIN, 50);
SimpleButton nextButton(NEXT_BUTTON_PIN, 50);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
    U8G2_R0,
    U8X8_PIN_NONE,
    DISPLAY_SCL,
    DISPLAY_SDA);

void setup()
{
  Serial.begin(115200);

  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);

  u8g2.begin();

  Keyboard.begin();
  playPauseButton.begin();
  prevButton.begin();
  nextButton.begin();

  Serial.println("NebulaKey firmware started");

  prev_CLK_state = digitalRead(CLK_PIN);
}

void loop()
{
  prevButton.update();
  nextButton.update();
  playPauseButton.update();

  if (playPauseButton.wasPressed())
  {
    Serial.println("Play/Pause");
    Keyboard.consumerPress(KEY_PLAY_PAUSE);
    Keyboard.consumerRelease();
  }

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

  if (prevButton.wasPressed())
  {
    Serial.println("Previous button pressed");
    Keyboard.consumerPress(KEY_SCAN_PREVIOUS);
    Keyboard.consumerRelease();
  }

  if (nextButton.wasPressed())
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

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);

  uint16_t screenWidth = u8g2.getWidth();
  uint16_t screenHeight = u8g2.getHeight();

  const char *text = currentTrack.c_str();
  int textWidth = u8g2.getStrWidth(text);

  int x = (screenWidth - textWidth) / 2;
  int y = screenHeight / 2;

  u8g2.drawStr(x, y, text);

  // -------- Media bar --------
  int iconY = screenHeight - ICON_8_HEIGHT;

  int prevX = 8;
  int playX = (screenWidth - ICON_8_WIDTH) / 2;
  int nextX = screenWidth - ICON_8_WIDTH - 8;

  u8g2.drawXBMP(prevX, iconY, ICON_8_WIDTH, ICON_8_HEIGHT, icon_prev_8x8);
  u8g2.drawXBMP(playX, iconY, ICON_8_WIDTH, ICON_8_HEIGHT, icon_play_8x8);
  u8g2.drawXBMP(nextX, iconY, ICON_8_WIDTH, ICON_8_HEIGHT, icon_next_8x8);

  u8g2.sendBuffer();

  u8g2.sendBuffer();
}
