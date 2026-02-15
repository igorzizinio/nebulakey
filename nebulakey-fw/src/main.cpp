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
volatile char currentTrack[32] = "No track playing";
volatile uint64_t currentTime = 0;
volatile uint64_t endTime = 0;
// the current software send in SECONDS, so uint64 is big overkill, BUT i want to make sure this never overflows (rp2040 has no problems with mem anyway)
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

  // basicamente pegamos varias leituras (pq o encoder manda MUITossss valores de uma vez, meio flutuantes e incertos), e fazemos uma média pra ter mais precisão noq realmente aconteceu!
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

    if (line == "HELLO")
    {
      Serial.println("HOWDY");
    }

    else if (line.startsWith("TRACK: "))
    {
      strncpy((char *)currentTrack, line.substring(7).c_str(), sizeof(currentTrack) - 1);
      currentTrack[sizeof(currentTrack) - 1] = '\0'; // garante null terminator
      Serial.println("OK");
    }
    else if (line.startsWith("TIMELINE: "))
    {
      String timeline = line.substring(10);

      int slashIndex = timeline.indexOf("/");
      if (slashIndex < 0)
      {
        Serial.println("INVALID TIMELINE");
        return;
      }

      String currentStr = timeline.substring(0, slashIndex);
      String endStr = timeline.substring(slashIndex + 1);

      currentTime = currentStr.toInt();
      endTime = endStr.toInt();

      Serial.println("OK");
    }
    else
    {
      Serial.println("invalid input");
    }
  }
}

// executa coisas relacionadas ao display no outro core (i2c pesado), pra n acabar travando o encoder
// ! LEMBRAR SEMPRE: usar valores thread-safe (uint8_t) com volatile
void loop1()
{
  static unsigned long lastDisplayUpdate = 0;
  static bool displayReady = false;
  static char lastTrack[32] = "";
  static uint16_t lastTime = 0;
  static uint16_t lastEnd = 0;
  static unsigned long lastHeartbeat = 0;

  if (!displayReady)
  {
    u8g2.begin();
    u8g2.setPowerSave(0);
    u8g2.setBusClock(400000);
    displayReady = true;
  }

  // Atualiza quando os dados mudam, com um heartbeat lento para evitar tela “travada”
  if (millis() - lastDisplayUpdate > 33)
  {
    lastDisplayUpdate = millis();

    // ==== Copia valores thread-safe de variáveis voláteis ====
    char trackCopy[32];
    uint64_t timeCopy, endCopy;

    noInterrupts(); // opcional no RP2040 para evitar leitura parcialmente escrita
    strncpy(trackCopy, (const char *)currentTrack, sizeof(trackCopy) - 1);
    trackCopy[sizeof(trackCopy) - 1] = '\0';
    timeCopy = currentTime;
    endCopy = endTime;
    interrupts();

    bool changed = (strncmp(trackCopy, lastTrack, sizeof(lastTrack)) != 0) ||
                   (timeCopy != lastTime) ||
                   (endCopy != lastEnd);

    if (!changed && (millis() - lastHeartbeat < 500))
      return;

    lastHeartbeat = millis();
    strncpy(lastTrack, trackCopy, sizeof(lastTrack) - 1);
    lastTrack[sizeof(lastTrack) - 1] = '\0';
    lastTime = timeCopy;
    lastEnd = endCopy;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    uint16_t screenWidth = u8g2.getWidth();
    uint16_t screenHeight = u8g2.getHeight();

    // ==== Progress bar no topo ====
    int filledPixels = 0;
    if (endCopy > 0)
    {
      filledPixels = (int)((float)timeCopy / endCopy * (screenWidth - 4));
      if (filledPixels < 0)
        filledPixels = 0;
      if (filledPixels > (int)(screenWidth - 4))
        filledPixels = screenWidth - 4;
    }
    u8g2.drawFrame(0, 0, screenWidth, 10); // borda da barra
    if (filledPixels > 0)
      u8g2.drawBox(1, 1, filledPixels, 8); // preenchimento

    // ==== Track name centralizado ====
    int textWidth = u8g2.getStrWidth(trackCopy);
    int x = (screenWidth - textWidth) / 2;
    int y = screenHeight / 2;
    u8g2.drawStr(x, y, trackCopy);

    // ==== Ícones no rodapé ====
    int iconY = screenHeight - ICON_8_HEIGHT;
    int prevX = 8;
    int playX = (screenWidth - ICON_8_WIDTH) / 2;
    int nextX = screenWidth - ICON_8_WIDTH - 8;

    u8g2.drawXBMP(prevX, iconY, ICON_8_WIDTH, ICON_8_HEIGHT, icon_prev_8x8);
    u8g2.drawXBMP(playX, iconY, ICON_8_WIDTH, ICON_8_HEIGHT, icon_play_8x8);
    u8g2.drawXBMP(nextX, iconY, ICON_8_WIDTH, ICON_8_HEIGHT, icon_next_8x8);

    u8g2.sendBuffer();
  }
}
