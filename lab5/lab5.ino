#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic1 = NULL;
BLECharacteristic* pCharacteristic2 = NULL;
BLECharacteristic* pCharacteristic3 = NULL;
BLEDescriptor* pDescr1;
BLEDescriptor* pDescr2;
BLEDescriptor* pDescr3;
BLE2902* pBLE2902_1;
BLE2902* pBLE2902_2;
BLE2902* pBLE2902_3;

bool deviceConnected = false;
bool oldDeviceConnected = false;

int chunk_size = 500;

#define SERVICE_UUID        "7d94a961-d4d5-4193-8408-501e963b6368"
#define CHARACTERISTIC_UUID_1 "bab30471-5a4b-4090-9fe5-14423e36c24e"
#define CHARACTERISTIC_UUID_2 "bab30471-5a4b-4090-9fe5-14423e36c24f"
#define CHARACTERISTIC_UUID_3 "bab30471-5a4b-4090-9fe5-14423e36c250"

// Exhibition information
const char* title = "Halas Lace";
const char* text = "Lacing has more than 100 years of tradition in Kiskunhalas. This work is very time consuming and minutely. It demands one hundred percent manual work done with snow-white, very fine thread and an almost invisible needle. The secret of Halas lacing is known only to the Halas lace workers. The unique technique of lacing is passed on from generation to generation, and that knowledge is used by excellent folklore industrial artists of the lacing house to create wonders from thread in Kiskunhalas.";
const char* text2 = "Even though the technique is more than 110 years old, new motives are still applied in the laces each year, in which the tradition is mixed with the values of the present. The Halas lace work is not only a work of art, but also Hungary's ambassador in the world. Numerous international excellencies and aristocrats received Halas lacework as a gift from the Hungarian State.";
const char* text3 = "A few of these unique artistic pieces are owned by the Japanese Empress, Elisabeth II, the British Queen and the wife of US President Bush. John Paul II also received Halas lace work as a gift during his visit to Hungary in 1996.";

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device Connected");
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device Disconnected");
  }
};

void setup() {
  Serial.begin(9600);

  BLEDevice::init("ESP32 NHL Museum Beacon");
  BLEDevice::setMTU(chunk_size);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create three characteristics to send data
  pCharacteristic1 = pService->createCharacteristic(
    CHARACTERISTIC_UUID_1,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic2 = pService->createCharacteristic(
    CHARACTERISTIC_UUID_2,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic3 = pService->createCharacteristic(
    CHARACTERISTIC_UUID_3,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Create descriptors for each characteristic
  pDescr1 = new BLEDescriptor((uint16_t)0x2901);
  pDescr1->setValue("Exhibition Description: Halas Lace - Part 1");
  pCharacteristic1->addDescriptor(pDescr1);

  pDescr2 = new BLEDescriptor((uint16_t)0x2901);
  pDescr2->setValue("Exhibition Description: Halas Lace - Part 2");
  pCharacteristic2->addDescriptor(pDescr2);

  pDescr3 = new BLEDescriptor((uint16_t)0x2901);
  pDescr3->setValue("Exhibition Description: Halas Lace - Part 3");
  pCharacteristic3->addDescriptor(pDescr3);

  // Create separate notification descriptors for each characteristic
  pBLE2902_1 = new BLE2902();
  pBLE2902_1->setNotifications(true);
  pCharacteristic1->addDescriptor(pBLE2902_1);

  pBLE2902_2 = new BLE2902();
  pBLE2902_2->setNotifications(true);
  pCharacteristic2->addDescriptor(pBLE2902_2);

  pBLE2902_3 = new BLE2902();
  pBLE2902_3->setNotifications(true);
  pCharacteristic3->addDescriptor(pBLE2902_3);

  pService->start();

  // Start advertising the service
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0); // Set this to 0 if you want the default connection interval
  BLEDevice::setPower(ESP_PWR_LVL_N12);
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    // Send different parts of the text through each characteristic
    pCharacteristic1->setValue(String(text).c_str());
    pCharacteristic1->notify();

    pCharacteristic2->setValue(String(text2).c_str());
    pCharacteristic2->notify();

    pCharacteristic3->setValue(String(text3).c_str());
    pCharacteristic3->notify();

    delay(5000); // Delay between sending each part
  }

  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}
