#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "pitches.h"


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


const int melody[] = {
  NOTE_A4, REST, NOTE_B4, REST, NOTE_C5, REST, NOTE_A4, REST,
  NOTE_D5, REST, NOTE_E5, REST, NOTE_D5, REST,

  NOTE_G4, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_E5, NOTE_E5, REST,
  NOTE_D5, REST,

  NOTE_G4, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_D5, NOTE_D5, REST,
  NOTE_C5, REST, NOTE_B4, NOTE_A4, REST,

  NOTE_G4, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_C5, NOTE_D5, REST,
  NOTE_B4, NOTE_A4, NOTE_G4, REST, NOTE_G4, REST, NOTE_D5, REST, NOTE_C5, REST,

  NOTE_G4, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_E5, NOTE_E5, REST,
  NOTE_D5, REST,

  NOTE_G4, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_G5, NOTE_B4, REST,
  NOTE_C5, REST, NOTE_B4, NOTE_A4, REST,

  NOTE_G4, NOTE_A4, NOTE_C5, NOTE_A4, NOTE_C5, NOTE_D5, REST,
  NOTE_B4, NOTE_A4, NOTE_G4, REST, NOTE_G4, REST, NOTE_D5, REST, NOTE_C5, REST,

  NOTE_C5, REST, NOTE_D5, REST, NOTE_G4, REST, NOTE_D5, REST, NOTE_E5, REST,
  NOTE_G5, NOTE_F5, NOTE_E5, REST,

  NOTE_C5, REST, NOTE_D5, REST, NOTE_G4, REST
};

const int durations[] = {
  8, 8, 8, 8, 8, 8, 8, 4,
  8, 8, 8, 8, 2, 2,

  8, 8, 8, 8, 2, 8, 8,
  2, 8,

  8, 8, 8, 8, 2, 8, 8,
  4, 8, 8, 8, 8,

  8, 8, 8, 8, 2, 8, 8,
  2, 8, 4, 8, 8, 8, 8, 8, 1, 4,

  8, 8, 8, 8, 2, 8, 8,
  2, 8,

  8, 8, 8, 8, 2, 8, 8,
  2, 8, 8, 8, 8,

  8, 8, 8, 8, 2, 8, 8,
  4, 8, 3, 8, 8, 8, 8, 8, 1, 4,

  2, 6, 2, 6, 4, 4, 2, 6, 2, 3,
  8, 8, 8, 8,

  2, 6, 2, 6, 2, 1
};

const int MELODY_LENGTH = sizeof(melody) / sizeof(melody[0]);
int melodyIndex = 0;
unsigned long noteStartTime = 0;


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

void playAlarmMelody() {
  if (melodyIndex >= MELODY_LENGTH) {
    noTone(BUZZER);
    return;
  }

  int note = melody[melodyIndex];
  int noteDuration = 1000 / durations[melodyIndex];
  unsigned long now = millis();

  if (now - noteStartTime >= noteDuration) {
    noteStartTime = now;
    if (note != REST) {
      tone(BUZZER, note, noteDuration);
    } else {
      noTone(BUZZER);
    }
    melodyIndex++;
  }
}

void startAlarm() {
  alarmRinging = true;
  alarmTriggeredThisMinute = true;
  melodyIndex = 0;
  noteStartTime = 0;
  drawTime();
}

void stopAlarm() {
  alarmRinging = false;
  noTone(BUZZER);
  drawTime();
}

void checkAlarm() {
  if (alarmEditState > 0 || alarmRinging) {
    return;
  }

  if (hours == alarmHour && minutes == alarmMinute && !alarmTriggeredThisMinute) {
    startAlarm();
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


  if (lastSW1 == HIGH && sw1 == LOW) {
    lastButtonTime = now;
    if (alarmEditState == 1) {
      alarmHour = (alarmHour - 1 + 24) % 24;
      drawTime();
      drawAlarmInfo();
    } else if (alarmEditState == 2) {
      alarmMinute = (alarmMinute - 1 + 60) % 60;
      drawTime();
      drawAlarmInfo();
    }
  }


  if (lastSW2 == HIGH && sw2 == LOW) {
    lastButtonTime = now;
    if (alarmEditState == 1) {
      alarmHour = (alarmHour + 1) % 24;
      drawTime();
      drawAlarmInfo();
    } else if (alarmEditState == 2) {
      alarmMinute = (alarmMinute + 1) % 60;
      drawTime();
      drawAlarmInfo();
    }
  }


  if (lastSW3 == HIGH && sw3 == LOW) {
    lastButtonTime = now;
    if (!alarmRinging) {
      alarmEditState = (alarmEditState + 1) % 3;
      drawTime();
    }
  }


  if (lastSW4 == HIGH && sw4 == LOW) {
    lastButtonTime = now;
    if (alarmEditState > 0) {
      alarmEditState = 0;
      drawTime();
    } else if (alarmRinging) {
      stopAlarm();
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

  if (alarmRinging) {
    playAlarmMelody();
  }

  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = millis();
    drawTime();
    drawAlarmInfo();
  }
}