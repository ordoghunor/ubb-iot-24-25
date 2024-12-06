#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLEDescriptor* pDescr;
BLE2902* pBLE2902;

bool deviceConnected = false;
bool oldDeviceConnected = false;

int chunk_size = 500;

#define SERVICE_UUID        "7d94a961-d4d5-4193-8408-501e963b6368"
#define CHARACTERISTIC_UUID "bab30471-5a4b-4090-9fe5-14423e36c24e"

// Exhibition information
const char* title = "Halas Lace";
const char* text = "Lacing has more than 100 years of tradition in Kiskunhalas. This work is very time consuming and minutely. It demands one hundred percent manual work done with snow-white, very fine thread and an almost invisible needle. The secret of Halas lacing is known only to the Halas lace workers. The unique technique of lacing is passed on from generation to generation, and that knowledge is used by excellent folklore industrial artists of the lacing house to create wonders from thread in Kiskunhalas. Even though the technique is more than 110 years old, new motives are still applied in the laces each year, in which the tradition is mixed with the values of the present. The Halas lace work is not only a work of art, but also Hungary’s ambassador in the world. Numerous international excellencies and aristocrats received Halas lacework as a gift from the Hungarian State. A few of these unique artistic pieces are owned by the Japanese Empress, Elisabeth II, the British Queen and the wife of US President Bush. John Paul II also received Halas lace work as a gift during his visit to Hungary in 1996.";

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

  // Create a characteristic to notify data
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Add a descriptor for the characteristic
  pDescr = new BLEDescriptor((uint16_t)0x2901);
  pDescr->setValue("Exhibition Description: Halas Lace");
  pCharacteristic->addDescriptor(pDescr);

  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  pCharacteristic->addDescriptor(pBLE2902);

  pService->start();

  // Start advertising the service
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0); // Set this to 0 if you want the default connection interval
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    // Send the text in smaller chunks to avoid BLE packet size limit
    sendExhibitionText(text);
    delay(5000); 
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

void sendExhibitionText(const char* text) {
  int textLength = strlen(text);
  int start = 0;

  while (start < textLength) {
    // Send the chunk as a string and ensure it fits within the chunk size limit
    String chunk = String(text).substring(start, start + chunk_size);
    pCharacteristic->setValue(chunk.c_str());
    pCharacteristic->notify();
    
    start += chunk_size;
    delay(5000); // Delay to prevent overwhelming the connection
  }
}
