#include <ArduinoBLE.h>
#include <Arduino_HTS221.h> // Temperature & Humidity
#include <Arduino_LSM9DS1.h> // Magnetometer

#define LEDR (22u)
#define LEDG (23u)
#define LEDB (24u)

#define INTERVAL 60000 // ms

// #define DEBUG true // Comment this out to compile code for production

// // Replacement class for Serial
// #if !DEBUG
// class NullSerialClass
// {
//   public:
//   void begin(int speed) {}
//   void println(int xMag, int type=DEC) {}
//   void println(const char *p=NULL) {}
//   void println(const String p) {}
//   void print(int xMag, int type=DEC) {}
//   void print(const char *p) {}
//   void print(const String p) {}
//   int available() {return 0;}
//   int read() {return -1;}
//   void flush() {}
// } NullSerial;
// #define Serial NullSerial
// #endif

// Device info
const char* nameOfPeripheral = "Arduino Nano 33 BLE Sense";
const char* uuidOfService = "00001101-0000-1000-8000-00805f9b34fb";
const char* uuidOfReadChar = "00001142-0000-1000-8000-00805f9b34fb";
const char* uuidOfWriteChar = "00001143-0000-1000-8000-00805f9b34fb";

// BLE Service
BLEService IMUService(uuidOfService);

// Setup the incoming data characteristic.
const int WRITE_BUFFER_SIZE = 256;
bool WRITE_BUFFER_FIXED_LENGTH = false;

// Read / Write Characteristics
BLECharacteristic readChar(uuidOfReadChar, BLEWriteWithoutResponse | BLEWrite, WRITE_BUFFER_SIZE, WRITE_BUFFER_FIXED_LENGTH);
BLECharacteristic writeChar(uuidOfWriteChar, BLERead | BLENotify | BLEBroadcast, "123456789123456789123456789123456789123456789123456789123456789123456789");

bool isOpen = false;
unsigned long lastRefreshTime = 0;

void setup() {
  // Serial.begin(9600);
  // while (!Serial);
  // Serial.println("Started");

  // Initialize LED pins
  initLED();

  // Initialize IMU
  initIMU();

  // Initialize HTS
  initHTS();

  // Initialize BLE
  initBLE();
}

void loop() {
  BLEDevice central = BLE.central();

  // Door
  if (IMU.magneticFieldAvailable()) {
    float xMag, yMag, zMag;

    IMU.readMagneticField(xMag, yMag, zMag);

    if (zMag < 350 && !isOpen) {
      showRedLight();
      // Serial.println("Door open");
      writeChar.writeValue("notifications: Door has been opened");
      isOpen = true;
    } else if (zMag >= 350 && isOpen) {
      turnOffLED();
      // Serial.println("Door closed");
      writeChar.writeValue("notifications: Door has been closed");
      isOpen = false;
    }
  }

  // Humidity, temperature, and pressure
  if(millis() - lastRefreshTime > INTERVAL) {
    lastRefreshTime = millis();

    float temperature = HTS.readTemperature();
    float humidity = HTS.readHumidity();

    String temperatureString = String("temperature: ") + String(temperature, 0);
    String humidityString = String("humidity: ") + String(humidity, 0);

    writeChar.writeValue(temperatureString.c_str());
    writeChar.writeValue(humidityString.c_str());
  }
}

void initIMU() {
  if (!IMU.begin()) {
    // Serial.println("Failed to initialize IMU!");
    while (1);
  }
}

void initHTS() {
  if (!HTS.begin()) {
    // Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }
}

void initLED() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  turnOffLED();
}

void initBLE() {
  if (!BLE.begin()) {
    // Serial.println("Failed to initialize BLE!");
    while (1);
  }

  // Create BLE service and characteristics.
  BLE.setLocalName(nameOfPeripheral);
  BLE.setAdvertisedService(IMUService);
  IMUService.addCharacteristic(readChar);
  IMUService.addCharacteristic(writeChar);
  BLE.addService(IMUService);

  // Bluetooth LE connection handlers.
  BLE.setEventHandler(BLEConnected, onBLEConnected);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);

  // Event driven reads.
  readChar.setEventHandler(BLEWritten, onGetData);

  // Let's tell devices about us.
  BLE.advertise();

  // Print out full UUID and MAC address.
  // Serial.println("Peripheral advertising info: ");
  // Serial.print("Name: ");
  // Serial.println(nameOfPeripheral);
  // Serial.print("MAC: ");
  // Serial.println(BLE.address());
  // Serial.print("Service UUID: ");
  // Serial.println(IMUService.uuid());
  // Serial.print("readCharacteristic UUID: ");
  // Serial.println(uuidOfReadChar);
  // Serial.print("writeCharacteristic UUID: ");
  // Serial.println(uuidOfWriteChar);


  // Serial.println("Bluetooth device active, waiting for connections...");

  showYellowLight();
}



/*
  BLE
*/

void onGetData(BLEDevice central, BLECharacteristic characteristic) {
  byte bytes[256];
  int bytesLength = readChar.readValue(bytes, 256);

  String command = "";
  for (int i = 0; i < bytesLength; i++) {
    command += (char)bytes[i];
  }

  // Serial.print("Received Command: ");
  // Serial.println(command);

  if(command == "verify_connection") {
    writeChar.writeValue("y");
  }
}

void onBLEConnected(BLEDevice central) {
  // Serial.print("Connected event, central: ");
  // Serial.println(central.address());
  showGreenLight();
}

void onBLEDisconnected(BLEDevice central) {
  // Serial.print("Disconnected event, central: ");
  // Serial.println(central.address());
  showYellowLight();
}



/*
  LED
*/
void showGreenLight() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, HIGH);
}

void showYellowLight() {
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, HIGH);
}

void showRedLight() {
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
}

void blinkLight(String color) {
  if (color == "green") {
    showGreenLight();
  } else if (color == "red") {
    showRedLight();
  }
  
  delay(1000);

  turnOffLED();
}

void turnOffLED() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
}
