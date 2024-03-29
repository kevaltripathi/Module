#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "esp_wpa2.h"
#include <WiFi.h>
#include <HTTPClient.h>

#include "EEPROM.h"

#include <MD_MAX72xx.h>
#include <SPI.h>

#include <Adafruit_ssd1306syp.h>


#define SERVICE_UUID "82adecb0-1fca-4f1e-a217-de9585be8a13"
#define TITLE_CHARACTERISTIC_UUID "82adecb1-1fca-4f1e-a217-de9585be8a13"
#define MESSAGE_CHARACTERISTIC_UUID "82adecb2-1fca-4f1e-a217-de9585be8a13"
#define MATRIX_CHARACTERISTIC_UUID "82adecb3-1fca-4f1e-a217-de9585be8a13"
#define SSID_CHARACTERISTIC_UUID "82adecb4-1fca-4f1e-a217-de9585be8a13"
#define PW_CHARACTERISTIC_UUID "82adecb5-1fca-4f1e-a217-de9585be8a13"

#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 1

bool deviceConnected = false;
bool written = false;
bool sleeping = false;

bool titleWrite = false;
bool messageWrite = false;
bool matrixWrite = false;
bool ssidWrite = false;
bool pwWrite = false;

const int buttonPin = 35;
int currentButton;
int previousButton;

MD_MAX72XX grid = MD_MAX72XX(HARDWARE_TYPE, SS, MAX_DEVICES);
Adafruit_ssd1306syp display(SDA,SCL);

#define EEPROM_SIZE 98

String title = "";
String message = "";
String matrix = "";
String ssid = "";
String pw = "";

StaticJsonDocument<192> dots;
StaticJsonDocument<384> doc;

unsigned long elapsed = 0;
unsigned long refreshTime = 0;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    Serial.println("connected");
    deviceConnected = true;
  }
  void onDisconnect(BLEServer *pServer) override {
    Serial.println("disconnected");
    deviceConnected = false;
    written = false;
    pServer->getAdvertising()->start();
  }
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
public:
  void onWrite(BLECharacteristic *chareristic, esp_ble_gatts_cb_param_t *param) override {
    if (chareristic->getUUID().toString() == TITLE_CHARACTERISTIC_UUID) {
      title = chareristic->getValue().c_str();
      Serial.println("title: " + title);
      titleWrite = true;
    } else if (chareristic->getUUID().toString() == MESSAGE_CHARACTERISTIC_UUID) {
      message = chareristic->getValue().c_str();
      DeserializationError error = deserializeJson(doc, message);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }  
      Serial.println("message: " + message);
      messageWrite = true;
    } else if (chareristic->getUUID().toString() == MATRIX_CHARACTERISTIC_UUID) {
      matrix = chareristic->getValue().c_str();
      DeserializationError error = deserializeJson(dots, matrix);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }  
      Serial.println("matrix: " + matrix);
      matrixWrite = true;
    } else if (chareristic->getUUID().toString() == SSID_CHARACTERISTIC_UUID) {
      ssid = chareristic->getValue().c_str();
      Serial.println("ssid: " + ssid);
      ssidWrite = true;
    } else if (chareristic->getUUID().toString() == PW_CHARACTERISTIC_UUID) {
      pw = chareristic->getValue().c_str();
      Serial.println("pw: " + pw);
      pwWrite = true;
    }
    written = true;
    Serial.println("written");
  }
};

String apiGet(String path) {
  HTTPClient http;

  http.begin("http://api.DOMAIN/" + path);

  int httpCode = http.GET();
  
  if(httpCode > 0) {
    if(httpCode == HTTP_CODE_OK) {
      return http.getString();
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

void apiPost(String path, String data) {
  HTTPClient http;

  http.begin("http://api.DOMAIN/" + path);
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST(data);
  
  if(httpCode > 0) {
    Serial.println(http.getString());
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

void lcd() {
  display.clear();
  if (!sleeping) {
    int lineQty = doc["lineQty"];
    JsonArray lines = doc["lines"];
    JsonArray weight = doc["weight"];

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(title);

    int last_line = 16;
    for (int x = 0; x < lineQty; x++) {
      if (last_line > 63) {
        break;
      }
      display.setTextColor(WHITE);
      display.setTextSize(weight[x]);
      display.setCursor(0,last_line);
      display.println(lines[x].as<String>()); 
      last_line += 8 * weight[x].as<int>();
    }
  }
  display.update();
}

void dotMatrix() {
  grid.clear();
  if (!sleeping) {
    for (int i = 0; i <= 7; i++) {
      uint8_t row = dots[i].as<int>();
      grid.setRow(i, row);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(buttonPin, INPUT);
  currentButton = digitalRead(buttonPin);

  BLEDevice::init("Module BLE");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *titleChareristic = pService->createCharacteristic(
    TITLE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  titleChareristic->setValue("0");
  titleChareristic->setCallbacks(new CharacteristicCallbacks());

  BLECharacteristic *messageCharacteristic = pService->createCharacteristic(
    MESSAGE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  messageCharacteristic->setValue("0");
  messageCharacteristic->setCallbacks(new CharacteristicCallbacks());

  BLECharacteristic *matrixCharacteristic = pService->createCharacteristic(
    MATRIX_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  matrixCharacteristic->setValue("0");
  matrixCharacteristic->setCallbacks(new CharacteristicCallbacks());

  BLECharacteristic *ssidCharacteristic = pService->createCharacteristic(
    SSID_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  ssidCharacteristic->setValue("0");
  ssidCharacteristic->setCallbacks(new CharacteristicCallbacks());

  BLECharacteristic *pwCharacteristic = pService->createCharacteristic(
    PW_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pwCharacteristic->setValue("0");
  pwCharacteristic->setCallbacks(new CharacteristicCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to init EEPROM");
  } else {
    ssid = "";
    int ssidLength = int(EEPROM.read(0));
    for (int i = 1; i <= ssidLength; i++) {
      ssid += char(EEPROM.read(i));
    }

    pw = "";
    int pwLength = int(EEPROM.read(32));
    for (int i = 1; i <= pwLength; i++) {
      pw += char(EEPROM.read(i + 32));
    }

  }

  delay(10000);

  while (deviceConnected) {
    delay(100);
  }

  if (ssidWrite && pwWrite) {
    if (!EEPROM.begin(EEPROM_SIZE)) {
      Serial.println("failed to init EEPROM");
    } else {
      EEPROM.write(0, ssid.length());
      int addr = 1;
      for (int i = 0; i < ssid.length(); i++) {
        EEPROM.write(addr, ssid[i]);
        addr += 1;
      }

      EEPROM.write(33, pw.length());
      addr = 34;
      for (int i = 0; i < pw.length(); i++) {
        EEPROM.write(addr, pw[i]);
        addr += 1;
      }

      EEPROM.commit();
    }
  }

  ssidWrite = false;
  pwWrite = false;

  char s[ssid.length() + 1];
  char p[pw.length() + 1];
  
  ssid.toCharArray(s, ssid.length() + 1);
  pw.toCharArray(p, pw.length() + 1);
  
  Serial.println("Mac Address: " + WiFi.macAddress());
  Serial.print("Waiting for WiFi...");

  WiFi.disconnect(true);
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME, strlen(EAP_USERNAME));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)p, sizeof(p));
  esp_wifi_sta_wpa2_ent_enable();  
  WiFi.begin(s);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  grid.begin();
  grid.control(MD_MAX72XX::INTENSITY, 0);

  display.initialize();
  display.clear();
}

void loop() {

  previousButton = currentButton;

  if (deviceConnected) {
    if (written) {
      currentButton = digitalRead(buttonPin);
      if (previousButton == HIGH && currentButton == LOW) {
        if (titleWrite) {
          apiPost("title", title);
          titleWrite = false;
        }
        if (messageWrite) {
          apiPost("message", message);
          messageWrite = false;
        }
        if (matrixWrite){
          apiPost("matrix", matrix);
          matrixWrite = true;          
        } 
        if (ssidWrite && pwWrite) {
          if (!EEPROM.begin(EEPROM_SIZE)) {
            Serial.println("failed to init EEPROM");
          } else {
            EEPROM.write(0, ssid.length());
            int addr = 1;
            for (int i = 0; i < ssid.length(); i++) {
              EEPROM.write(addr, ssid[i]);
              addr += 1;
            }

            EEPROM.write(32, pw.length());
            addr = 33;
            for (int i = 0; i < pw.length(); i++) {
              EEPROM.write(addr, pw[i]);
              addr += 1;
            }

            EEPROM.commit();
            ssidWrite = false;
            pwWrite = false;
          }
        }
        written = false;
        sleeping = false;
      }
    }
  } else {
    if (millis() - elapsed > 10000) {
      title = apiGet("title");

      message = apiGet("message");
      DeserializationError error = deserializeJson(doc, message);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      matrix = apiGet("matrix");
      error = deserializeJson(dots, matrix);  

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }        

      elapsed = millis();
    }

    currentButton = digitalRead(buttonPin);
    if (previousButton == HIGH && currentButton == LOW) {
      sleeping = !sleeping;
      Serial.println("sleeping: " + String(sleeping));
    }
  }

  if (millis() - refreshTime > 1000) {
    lcd();
    dotMatrix();
    refreshTime = millis();
  }
}