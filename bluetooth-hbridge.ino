/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.
   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic * pCharacteristic;
bool deviceConnected = false;
float txValue = 0;
const int readPin = 32; // Use GPIO number. See ESP32 board pinouts
const int LED = 2; 
/* Could be different depending on the dev board. I used the DOIT ESP32 dev board.
//std::string rxValue; // Could also make this a global var to access it in loop()
// See the following for generating UUIDs: https://www.uuidgenerator.net/
*/
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

int Pwr = 3;

class HBridgeMaderController {
  private:
    int pinArray[4] = { // The pins used as output
      13, // motor 1 pin A
      12, // motor 1 pin B
      14, // motor 2 pin A
      27  // motor 2 pin B
    };
    int direction[5][4] = {
      {1, 0, 1, 0},    // Forward =0
      {0, 1, 0, 1},    // Backwords =1
      {1, 0, 0, 0},    // turn left =3
      {0, 0, 1, 0},    // Turn right = 4
      {0, 0, 0, 0},    // Standby =2
    };
    int pinCount = 4; // Pins uses in array
    const int Low = 127;
    const int Med = 191;
    const int High = 255;
   
  public:
    HBridgeMaderController() {
      for (int count = 0; count <= pinCount; count++)
      {
          ledcSetup(count, 5000, 8);
          ledcAttachPin(this->pinArray[count], count);
          ledcWrite(this->pinArray[count], 0);
      }
    }

    void drive(int x, int drive_pwr) { //Driveing the pins off of the input of x.
        for (int i = 0; i < this->pinCount; i++)
        {
            if (this->direction[x][i] == 1)
            {
                ledcWrite(i, this->power_settings[drive_pwr]);
            }
            else
            {
                ledcWrite(i, 0);
            }
        }
    }
};

HBridgeMaderController motor_controller;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer * pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer * pServer) {
    deviceConnected = false;
  }
};

int serial_state = 53;
int state = 2;

class MyCallbacks: public BLECharacteristicCallbacks {

  void onWrite(BLECharacteristic * pCharacteristic) {
    std::string rxValue = pCharacteristic -> getValue();

    if (rxValue.length() > 0) {
      if (rxValue.length() > 6) {

        serial_state = rxValue[5]; // state is askii!
        if(serial_state > 53){
        switch (serial_state) {
            case 54:  Serial.println("serial_state Low");   Pwr = Low;  break;
            case 55:  Serial.println("serial_state Med");   Pwr = Med;  break;
            case 56:  Serial.println("serial_state High");  Pwr = High; break;
            default:  Serial.println("serial_state High");  Pwr = High;       }
        switch (serial_state) {
        case 49:  Serial.println("serial_state Forward");  state = 0;  break;
        case 50:  Serial.println("serial_state Back");     state = 1;  break;
        case 51:  Serial.println("serial_state Left");     state = 2;  break;
        case 52:  Serial.println("serial_state Right");    state = 3;  break;
        case 53:  Serial.println("serial_state STOPED");   state = 4;  break;
        default:  state = 4;        }
        motor_controller.drive(state, Pwr);
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  // Create the BLE Device
  BLEDevice::init("ESP32 UART Test"); // Give it a name
  // Create the BLE Server
  BLEServer * pServer = BLEDevice::createServer();
  pServer -> setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService * pService = pServer -> createService(SERVICE_UUID);
  // Create a BLE Characteristic
  pCharacteristic = pService -> createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic -> addDescriptor(new BLE2902());
  BLECharacteristic * pCharacteristic = pService -> createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic -> setCallbacks(new MyCallbacks());
  // Start the service
  pService -> start();
  // Start advertising
  pServer -> getAdvertising() -> start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    // Fabricate some arbitrary junk for now...
    txValue = analogRead(readPin) / 3.456; 
    // This could be an actual sensor reading!
    // Let's convert the value to a char array:
    char txString[8]; // make sure this is big enuffz
    dtostrf(txValue, 1, 2, txString); 
    // float_val, min_width, digits_after_decimal, char_buffer
    //    pCharacteristic->setValue(&txValue, 1); // To send the integer value
    //    pCharacteristic->setValue("Hello!"); // Sending a test message
    pCharacteristic -> setValue(txString);

    pCharacteristic -> notify(); 
    /*Send the value to the app!
    Serial.print("*** Sent Value: ");
    Serial.print(txString);
    Serial.println(" ***");
    You can add the rxValue checks down here instead
    if you set "rxValue" as a global var at the top!
    Note you will have to delete "std::string" declaration
    of "rxValue" in the callback function.
    if (rxValue.find("A") != -1) { 
    Serial.println("Turning ON!");
    digitalWrite(LED, HIGH);
    }
    else if (rxValue.find("B") != -1) {
    Serial.println("Turning OFF!");
    digitalWrite(LED, LOW);
    }*/
  }
  delay(1000);
}
