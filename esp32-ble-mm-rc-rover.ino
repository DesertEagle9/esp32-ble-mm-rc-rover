/*********
  Rui Santos
  Complete instructions at:
  https://RandomNerdTutorials.com/esp32-ble-server-client/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy 
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in 
  all copies or substantial portions of the Software.

  Royce Danam
  Modified original code to connect an ESP32 BLE Client to the Micromelon 
  Rover BLE Server to read the rover heartbeat and move the rover motors using
  physical buttons connected to the GPIO pins on the ESP32
*********/

// Libraries
#include "BLEDevice.h"
#include <ezButton.h>

//BLE Server name on the rover
#define bleServerName "Micromelon0037"

// Definitions to index buttonArray
#define FORWARD 0
#define REVERSE 1
#define PIVOT_LEFT 2
#define PIVOT_RIGHT 3

// ESP32 GPIO pins connected to four buttons on a breadboard
const int BUTTON_NUM = 4;
const int BUTTON_PIN_1 = 22;
const int BUTTON_PIN_2 = 21;
const int BUTTON_PIN_3 = 17;
const int BUTTON_PIN_4 = 16;

// Array of ezButton objects
ezButton buttonArray[] = {
  ezButton(BUTTON_PIN_1), 
  ezButton(BUTTON_PIN_2), 
  ezButton(BUTTON_PIN_3), 
  ezButton(BUTTON_PIN_4)
};

/* BLE communication packets */
// packet to send over BLE connection to move rover motors forward
uint8_t moveForwardPacket[] = {1, 0, 7, 42, 42, 0, 0, 0, 0, 0}; 
// packet to send over BLE connection to move rover motors backward
uint8_t moveBackwardPacket[] = {1, 0, 7, 214, 214, 0, 0, 0, 0, 0}; 
// packet to send over BLE connection to pivot rover counter-clockwise
uint8_t pivotLeftPacket[] = {1, 0, 7, 214, 42, 0, 0, 0, 0, 0}; 
// packet to send over BLE connection to pivot rover clockwise
uint8_t pivotRightPacket[] = {1, 0, 7, 42, 214, 0, 0, 0, 0, 0}; 
// packet to send over BLE connection to stop rover motors
uint8_t stopPacket[] = {1, 0, 7, 0, 0, 0, 0, 0, 0, 0}; 

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID roverServiceUUID("00000001-0000-1000-8000-00805f9b34fb");

// Hearbeat Characteristic
static BLEUUID heartbeatCharacteristicUUID("00001112-0000-1000-8000-00805f9b34fb");
static BLEUUID uartCharacteristicUUID("00001111-0000-1000-8000-00805f9b34fb");

// Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

// Address of the peripheral device. Address will be found during scanning
static BLEAddress *pServerAddress;
 
// Characteristic that we want to read
static BLERemoteCharacteristic* heartbeatCharacteristic;
static BLERemoteCharacteristic* uartCharacteristic;

// Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remote BLE Server
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server
  BLERemoteService* pRemoteService = pClient->getService(roverServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(roverServiceUUID.toString().c_str());
    return (false);
  }
 
  // Obtain a reference to the characteristics in the service of the remote 
  // BLE server.
  heartbeatCharacteristic = pRemoteService->getCharacteristic(heartbeatCharacteristicUUID);
  uartCharacteristic = pRemoteService->getCharacteristic(uartCharacteristicUUID);

  if (heartbeatCharacteristic == nullptr || uartCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }
  Serial.println(" - Found our characteristics");
  return true;
}

// Troubleshooting function to check if packets sent by ESP32 are received by 
// the rover
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

// Callback function when ESP32 has received rover's advertisement
class MyAdvertisedDeviceCallbacks: 
public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // Check if the name of the advertiser matches
    if (advertisedDevice.getName() == bleServerName) { 
      // Scan can be stopped, we found what we are looking for
      advertisedDevice.getScan()->stop(); 
      // Address of advertiser is the one we need
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true; // Set indicator, stating that we are ready to connect
      Serial.println("Device found. Connecting!");
    }
  }
};

// ESP32 to scan for available Bluetooth clients for connection
void initialiseBLEConnection() {
  Serial.println("Commencing BLE Scan and Pairing...");

  //Init BLE device
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed 
  // when we have detected a new device. Specify that we want active scanning 
  // and start the scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void establishBLEConnection() {
  // If the flag "doConnect" is true then we have scanned for and found the 
  // desired BLE Server with which we wish to connect.  Now we connect to it.
  // Once we are connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      
      // Troubleshooting: uncomment th line below to activate the 'notify' 
      // property of uartCharacteristic to check if button presses on the
      // rover are notified to the ESP32 BLE client listening on the 
      // uartCharacteristic 
      /* uartCharacteristic->registerForNotify(notifyCallback); */
     
      // initial read of rover hearbeat value. ESP32 BLE client must read 
      // rover heartbeat value every 4 seconds, else the rover will drop the //
      // BLE connection
      heartbeatCharacteristic->readValue(); 

      // Set rover to running state by writing a runningStatePacket to the 
      // uartCharacteristic
      uint8_t runningStatePacket[] = {1, 18, 1, 5};      
      uartCharacteristic->writeValue(runningStatePacket, 4, true);
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }
}

// Read the rover heartbeat value to maintain BLE connection between ESP32 and 
// rover
void readRoverHeartbeat() {
  if (connected) {
    heartbeatCharacteristic->readValue();

    // Check if ESP32 is reading rover's heartbeat
    /* Serial.println("reading heartbeat"); */
  }
}

// Set ezButton debounce time for breadboard pushbuttons
void setDebounce() {
  for(byte i = 0; i < BUTTON_NUM; i++){
    buttonArray[i].setDebounceTime(50); // set debounce time to 50 milliseconds
  }
}

// MUST call the loop() function first for ezButton to work
void loopButtonArray() {
  for(byte i = 0; i < BUTTON_NUM; i++)
    buttonArray[i].loop(); 
}

// Main loop to move rover by sending corresponding BLE packets
void moveRover() {
   if (connected) {
      if (buttonArray[FORWARD].isPressed()) {
        Serial.println("FORWARD button pressed");
        uartCharacteristic->writeValue(moveForwardPacket, 10, true);
        Serial.println("move forward");
      }
  
      if (buttonArray[REVERSE].isPressed()) {
        Serial.println("REVERSE button pressed");
        uartCharacteristic->writeValue(moveBackwardPacket, 10, true);
        Serial.println("move backward");
      }
  
      if (buttonArray[PIVOT_LEFT].isPressed()) {
        Serial.println("PIVOT_LEFT button pressed");
        uartCharacteristic->writeValue(pivotLeftPacket, 10, true);
        Serial.println("pivot counter-clockwise");
      }

      if (buttonArray[PIVOT_RIGHT].isPressed()) {
        Serial.println("PIVOT_RIGHT button pressed");
        uartCharacteristic->writeValue(pivotRightPacket, 10, true);
        Serial.println("pivot clockwise");
      }
    }    
    
    for(byte i = 0; i < BUTTON_NUM; i++) {
      if(buttonArray[i].isReleased()) {
        uartCharacteristic->writeValue(stopPacket, 10, true);
        Serial.println("stopping motors");
      }
  }
}

void setup() {  
  Serial.begin(115200);
  setDebounce();
  initialiseBLEConnection();
}

void loop() {  
  establishBLEConnection();
  readRoverHeartbeat();
  loopButtonArray();
  moveRover();
}
