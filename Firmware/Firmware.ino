#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#define SERVICE_UUID "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"

const int ledPin = LED_BUILTIN;
bool deviceConnected = false;
bool written = false;
bool led = false;

WiFiMulti WiFiMulti;
BLECharacteristic *pCharacteristic;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    Serial.println("connected");
    deviceConnected = true;
  }
  void onDisconnect(BLEServer *pServer) override {
    Serial.println("disconnected");
    deviceConnected = false;
  }
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
public:
  void onWrite(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param) override {
    led = pCharacteristic->getValue().compare("1") == 0;
    written = true;
  }
};

void bluetooth() {
  if (deviceConnected && written) {
    if (led) {
      Serial.println("LED on");
      digitalWrite(ledPin, HIGH);
      written = false;
    } else {
      Serial.println("LED off");
      digitalWrite(ledPin, LOW);
      written = false;
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  WiFiMulti.addAP("SSID", "PW");

  Serial.print("Waiting for WiFi... ");

  while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(ledPin, OUTPUT);

  BLEDevice::init("Module BLE");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setValue("0");
  pCharacteristic->setCallbacks(new CharacteristicCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

void loop() {

  bluetooth();
  
  HTTPClient http;

  http.begin("http://api.DOMAIN/light");

  int httpCode = http.GET();
  
  if(httpCode > 0) {
    Serial.println(httpCode);
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();

  Serial.println("Waiting 5 seconds before restarting...");
  delay(5000);
}