#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"

const int ledPin = LED_BUILTIN;
bool deviceConnected = false;
bool written = false;
bool led = false;

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

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Starting BLE work!");

  pinMode(ledPin, OUTPUT);

  BLEDevice::init("LED");
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
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
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