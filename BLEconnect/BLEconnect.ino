/*
   This sketch is a tiny example for you to scan BLE Devices and connect to desired one.
   Developer :  Gürdoğan Mert

   Based on Neil Kolban's example file: https://github.com/nkolban/ESP32_BLE_Arduino
*/

//================================ Libraries =====================================
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

//================================ Inputs / Outputs =====================================

#define deviceSelect_0 0            //Zeroth device selection button
#define deviceSelect_1 4            //first device selection button
#define scanButton    2                //scanning Button

// The remote service i wish to connect to.
static  BLEUUID serviceUUID(BLEUUID((uint16_t)0x180D));

static BLEScan* pBLEScan;                                           //BLE scan pointer
static BLEClient*  pClient;                                                //BLE client pointer
static BLEAdvertisedDevice _advertisedDevice[2];    //Let's take just two BLE devices as an example in this sketch

String deviceName[2];                                                     //found BLE device names

bool alreadyHasName = false;                                       //If device name already saved that bool will be true
bool haveDevice = false;

uint8_t scanTime = 5;                                                      //scaning time

//========================== Prototyping functions =============================

void scaningProgress();
void deviceSelect(uint8_t );

//========================== MyAdvertisedDeviceCallbacks =============================
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)   {

      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

      if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {        // The remote service i wish to connect to.
        for (uint8_t i = 0; i < sizeof(deviceName) / sizeof(deviceName[0]); i++)  {                                                              //Check routine this array for same device shouldn't stored again
          if (deviceName[i] == advertisedDevice.getName().c_str()) alreadyHasName = true;                                        //If same device found alreadyHasName->true
        }
        if (!alreadyHasName) {                                                                                                                                                //If diffrent device found store it
          for (uint8_t i = 0; i < sizeof(deviceName) / sizeof(deviceName[0]); i++) {
            if (deviceName[i] == "")    {
              deviceName[i] = advertisedDevice.getName().c_str();                                                                                     //Store name of BLE device into array
              _advertisedDevice[i] = advertisedDevice;                                                                                                          //Store current device
              break;
            }
          }
        }
        alreadyHasName = false;
      }
    }
}; // MyAdvertisedDeviceCallbacks

//================================    Setup   =================================
void setup()
{
  Serial.begin(115200); //
  Serial.println("Starting Esp32 Pico BLE Peripheral application...");

  pinMode(deviceSelect_0, INPUT_PULLUP);              //Zeroth
  pinMode(deviceSelect_1, INPUT_PULLUP);
  pinMode(scanButton, INPUT_PULLUP);

  BLEDevice::init("");

  pClient  = BLEDevice::createClient(); //created client
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}//setup

//================================    Loop   =================================
void loop()
{
  if (!digitalRead(scanButton))  scaningProgress();                //Scan function

  if (!digitalRead(deviceSelect_0)) deviceSelect(0);              //Zeroth device selection
  else if (!digitalRead(deviceSelect_1)) deviceSelect(1);      //first device selection
}//loop

void scaningProgress()
{
  memset(deviceName, 0, sizeof(deviceName));                 //clear DeviceName array
  if (pClient->isConnected()) pClient->disconnect();             //If our client has already connected disconnect from current device
  Serial.println("Scanning");
  pBLEScan->start(scanTime, false);                                     //Start scan
  Serial.println("Found Devices");
  for (uint8_t i = 0; i < sizeof(deviceName) / sizeof(deviceName[0]); i++)        //Found Devices stored in this array
  {
    Serial.print(i);
    Serial.print(".   Device Name:");
    Serial.println(deviceName[i]);
  }
}

void deviceSelect(uint8_t slaveNum)
{
  pBLEScan->stop();
  if (pClient->isConnected()) pClient->disconnect();  //If our client has already connected disconnect from current device
  if ( _advertisedDevice[slaveNum].haveServiceUUID())
  {
    Serial.println("Connecting");
    pClient->connect(&_advertisedDevice[slaveNum]);
    if (pClient->isConnected()) Serial.println("Connection Successfull");
    Serial.print("Connected \t");
    Serial.println(deviceName[slaveNum]);
  }
  else Serial.println("Device has not service or couldn't find");
}
