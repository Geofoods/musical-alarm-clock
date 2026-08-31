#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <Adafruit_ST7789.h> // driver for the ST7789 screen
#include <SPI.h>

// Defining pins for the display
#define TFT_SCLK 9
#define TFT_MOSI 10
#define TFT_RST 8
#define TFT_DC 6
#define TFT_CS 7
#define TFT_BL 21

#define SW1 5
#define SW2 4
#define SW3 3
#define SW4 2

#define BUZZER 20

// Fix setColRowStart() by exposing it via a subclass
class MyST7789 : public Adafruit_ST7789 {
public:
  MyST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
    : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {}

  void setOffsets(uint8_t col, uint8_t row) {
    _colstart = _colstart2 = col;
    _rowstart = _rowstart2 = row;
  }
};

MyST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

int hours = 12;
int minutes = 0;
int seconds = 0;

int alarmHour = 12;
int alarmMinute = 0;

// Alarm editing state machine: 0 = Normal Clock, 1 = Set Alarm Hours, 2 = Set Alarm Minutes
uint8_t alarmEditState = 0;

bool alarmRinging = false;
bool alarmTriggeredThisMinute = false;

bool lastSW1 = HIGH;
bool lastSW2 = HIGH;
bool lastSW3 = HIGH;
bool lastSW4 = HIGH;

unsigned long lastButtonTime = 0;
const unsigned long debounceTime = 150;

void drawTime() {
  tft.fillRect(0, 45, 320, 100, ST77XX_BLACK);

  if (alarmEditState > 0) {
    tft.setTextSize(2);
    tft.setCursor(95, 35);

    if (alarmEditState == 1) {
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("SET HOUR ");
    } else if (alarmEditState == 2) {
      tft.setTextColor(ST77XX_MAGENTA);
      tft.print("SET MINUTE");
    }

    tft.setTextSize(6);
    tft.setCursor(20, 65);

    // Highlight active edit parameter with cyan, leave inactive white
    if (alarmEditState == 1) tft.setTextColor(ST77XX_CYAN);
    else tft.setTextColor(ST77XX_WHITE);

    if (alarmHour < 10) tft.print("0");
    tft.print(alarmHour);

    tft.setTextColor(ST77XX_WHITE);
    tft.print(":");

    if (alarmEditState == 2) tft.setTextColor(ST77XX_CYAN);
    else tft.setTextColor(ST77XX_WHITE);

    if (alarmMinute < 10) tft.print("0");
    tft.print(alarmMinute);
  }
  else if (alarmRinging) {
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.setCursor(110, 35);
    tft.print("WAKE UP!");

    tft.setTextSize(6);
    tft.setCursor(20, 65);

    if (hours < 10) tft.print("0");
    tft.print(hours);
    tft.print(":");

    if (minutes < 10) tft.print("0");
    tft.print(minutes);
  }
  else {
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(6);
    tft.setCursor(20, 55);

    if (hours < 10) tft.print("0");
    tft.print(hours);
    tft.print(":");

    if (minutes < 10) tft.print("0");
    tft.print(minutes);
  }
}

void drawAlarmInfo() {
  tft.fillRect(0, 145, 320, 45, ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(75, 155);

  tft.print("ALARM ");

  if (alarmHour < 10) tft.print("0");
  tft.print(alarmHour);
  tft.print(":");

  if (alarmMinute < 10) tft.print("0");
  tft.print(alarmMinute);
}

void updateClock() {
  static unsigned long lastSecond = 0;
  unsigned long now = millis();

  if (now - lastSecond >= 1000) {
    lastSecond += 1000;
    seconds++;

    if (seconds >= 60) {
      seconds = 0;
      minutes++;
    }

    if (minutes >= 60) {
      minutes = 0;
      hours++;
    }

    if (hours >= 24) {
      hours = 0;
    }

    alarmTriggeredThisMinute = false;
  }
}

void checkAlarm() {
  if (alarmEditState > 0 || alarmRinging) {
    return;
  }

  if (hours == alarmHour && minutes == alarmMinute && !alarmTriggeredThisMinute) {
    alarmRinging = true;
    alarmTriggeredThisMinute = true;
    digitalWrite(BUZZER, HIGH);
    drawTime();
  }
}

void checkButtons() {
  bool sw1 = digitalRead(SW1);
  bool sw2 = digitalRead(SW2);
  bool sw3 = digitalRead(SW3);
  bool sw4 = digitalRead(SW4);

  unsigned long now = millis();

  if (now - lastButtonTime < debounceTime) {
    return;
  }

  // SW1: DECREMENT CURRENT PARAMETER
  if (lastSW1 == HIGH && sw1 == LOW) {
    lastButtonTime = now;
    if (alarmEditState == 1) { // Decrement Hour
      alarmHour = (alarmHour - 1 + 24) % 24;
      drawTime();
      drawAlarmInfo();
    } else if (alarmEditState == 2) { // Decrement Minute
      alarmMinute = (alarmMinute - 1 + 60) % 60;
      drawTime();
      drawAlarmInfo();
    }
  }

  // SW2: INCREMENT CURRENT PARAMETER
  if (lastSW2 == HIGH && sw2 == LOW) {
    lastButtonTime = now;
    if (alarmEditState == 1) { // Increment Hour
      alarmHour = (alarmHour + 1) % 24;
      drawTime();
      drawAlarmInfo();
    } else if (alarmEditState == 2) { // Increment Minute
      alarmMinute = (alarmMinute + 1) % 60;
      drawTime();
      drawAlarmInfo();
    }
  }

  // SW3: TOGGLE EDIT MODE (Off -> Hour -> Minute -> Off)
  if (lastSW3 == HIGH && sw3 == LOW) {
    lastButtonTime = now;
    if (!alarmRinging) {
      alarmEditState = (alarmEditState + 1) % 3;
      drawTime();
    }
  }

  // SW4: CANCEL / CONFIRM / STOP ALARM
  if (lastSW4 == HIGH && sw4 == LOW) {
    lastButtonTime = now;
    if (alarmEditState > 0) {
      alarmEditState = 0; // Exit edit mode
      drawTime();
    } else if (alarmRinging) {
      alarmRinging = false; // Silence alarm
      digitalWrite(BUZZER, LOW);
      drawTime();
    }
  }

  lastSW1 = sw1;
  lastSW2 = sw2;
  lastSW3 = sw3;
  lastSW4 = sw4;
}

void setup() {
  Serial.begin(115200);

  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);

  tft.init(76, 284);
  tft.setOffsets(82, 18);
  tft.invertDisplay(false);
  tft.setRotation(1);

  tft.fillScreen(ST77XX_BLACK);

  drawTime();
  drawAlarmInfo();
}

void loop() {
  updateClock();
  checkButtons();
  checkAlarm();

  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = millis();
    drawTime();
    drawAlarmInfo();
  }
}