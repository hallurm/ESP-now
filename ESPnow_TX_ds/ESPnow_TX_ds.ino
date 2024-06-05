#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

byte id = 1;
float temp = 0.00;
float humid = 0.00;
int battLvl = 0;
int tempBattLvl = 0;
int battPin = 34;
int sleepDuraion = 10; //Seconds
uint64_t sleepDurationFactor = 1000000;

Adafruit_BME280 bme;

uint8_t broadcastAddress[] = { 0x8C, 0xAA, 0xB5, 0x86, 0x15, 0xEC };

typedef struct struct_message {
  float a;  //temp
  float b;  //humid
  byte c;   //ID
  int d;    //Battery level 3V = 0% / 4.2V = 100%
  int e;  //Sleep duration in seconds
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}

void getBattLvl() {
  for (int i = 0; i < 10; i++) {
    tempBattLvl = tempBattLvl + analogRead(battPin);
    delay(10);
  }

  tempBattLvl = tempBattLvl / 10;
  battLvl = map(tempBattLvl, 0, 4095, 0, 100);
  battLvl = constrain(battLvl, 0, 100);
}

void setup() {
  //Serial.begin(115200);
  bme.begin(0x76);
  WiFi.mode(WIFI_STA);

  pinMode(battPin, INPUT);

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1,  // temperature
                  Adafruit_BME280::SAMPLING_X1,  // pressure
                  Adafruit_BME280::SAMPLING_X1,  // humidity
                  Adafruit_BME280::FILTER_OFF);

  bme.takeForcedMeasurement();
  temp = bme.readTemperature();
  humid = bme.readHumidity();

  if (esp_now_init() != ESP_OK) {
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    return;
  }

  getBattLvl();

  myData.a = temp;
  myData.b = humid;
  myData.c = id;
  myData.d = battLvl;
  myData.e = sleepDuraion;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  if (result != ESP_OK) {
    Serial.println("Fail");
  }

  delay(100);

  ESP.deepSleep((sleepDuraion*sleepDurationFactor));
}

void loop() {
}