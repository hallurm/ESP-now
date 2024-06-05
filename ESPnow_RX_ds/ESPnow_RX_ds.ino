#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "background.h"
#include "fonts.h"
#include "icons.h"

#define color_green 0x2502

String temp;
String humid;
String oldTemp;
String oldHumid;
String battLvl;
String oldBattLvl;

uint64_t sleepDuration = 0;
unsigned int connectionTimer = 0;

bool dataChanged = false;
bool linkChanged = false;
bool isLinked = false;
bool isNotLinked = true;

typedef struct struct_message {
  float a;  //temp
  float b;  //humid
  byte c;   //ID
  int d;    //Battery level 2.8V = 0% / 4.0V = 100%
  int e;    //Sleep duration in seconds
} struct_message;

struct_message myData;

TFT_eSPI tft = TFT_eSPI();

void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  oldTemp = temp;
  oldHumid = humid;
  oldBattLvl = battLvl;

  if (myData.c == 1) {
    temp = String(myData.a, 0);
    humid = String(myData.b, 0);
    battLvl = String(myData.d);
    sleepDuration = myData.e * 1000;
  }

  if ((temp != oldTemp) || humid != oldHumid || battLvl != oldBattLvl) {
    dataChanged = true;
  }

  // previousLinkState = currentLinkState;
  // currentLinkState = true;

  linkChanged = true;
  isLinked = true;
  isNotLinked = false;

  connectionTimer = millis();

}

void setup() {
  WiFi.mode(WIFI_STA);

  tft.init();
  tft.setRotation(2);
  tft.setFreeFont(&Orbitron_Medium_30);
  tft.setSwapBytes(true);

  buildBackground();

  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

  if (dataChanged) {

    tft.setTextColor(TFT_WHITE);
    tft.drawString(oldTemp, (120 - tft.textWidth(oldTemp) - 15), 75);
    tft.setTextColor(TFT_BLACK);
    tft.drawString(temp, (120 - tft.textWidth(temp) - 15), 75);

    tft.setTextColor(TFT_WHITE);
    tft.drawString(oldHumid + "%", 135, 75);
    tft.setTextColor(TFT_BLACK);
    tft.drawString(humid + "%", 135, 75);

    buildBattery();

    dataChanged = false;
  }

  buildLinked();
}

//---Functions---
void buildBackground() {
  tft.pushImage(0, 0, 240, 240, background);
  tft.pushImage((120 - 23 - 20), 30, 23, 40, temperature);
  tft.pushImage((120 + 20), 30, 29, 40, humidity);
}

void buildBattery() {
  tft.pushImage((120 + 20), 135, 40, 22, battery);
  if (battLvl.toInt() > 30) {
    tft.fillRect(146, 141, map(battLvl.toInt(), 0, 100, 0, 26), 10, color_green);
  } else {
    tft.fillRect(146, 141, map(battLvl.toInt(), 0, 100, 0, 26), 10, TFT_RED);
  }
}

void buildLinked() {

  if (millis() - connectionTimer > (sleepDuration * 1.2)) {
    isNotLinked = true;
    isLinked = false;
    linkChanged = true;
  }

  if (isLinked && linkChanged) {
    tft.fillRect((120 - 40 - 20), 135, 40, 32, TFT_WHITE);
    tft.pushImage((120 - 40 - 20), 135, 40, 20, connected);
    linkChanged = false;
  }

  if (!isLinked && linkChanged) {
    tft.fillRect((120 - 40 - 20), 135, 40, 20, TFT_WHITE);
    tft.pushImage((120 - 40 - 20), 135, 40, 32, not_connected);
    linkChanged = false;
    sleepDuration = 100000000;
  }

}