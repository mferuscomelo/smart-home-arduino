#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

#define LEDR (22u)
#define LEDG (23u)
#define LEDB (24u)

// #define DEBUG true // Comment this out to compile code for production

// Replacement class for Serial
#if !DEBUG
class NullSerialClass
{
  public:
  void begin(int speed) {}
  void println(int xMag, int type=DEC) {}
  void println(const char *p=NULL) {}
  void println(const String p) {}
  void print(int xMag, int type=DEC) {}
  void print(const char *p) {}
  void print(const String p) {}
  int available() {return 0;}
  int read() {return -1;}
  void flush() {}
} NullSerial;
#define Serial NullSerial
#endif

void setup() {
  Serial.begin(9600);

  // Initialize LED pins
  initLED();

  // Initialize IMU
  initIMU();

  Serial.print("Magnetic field sample rate = ");
  Serial.print(IMU.magneticFieldSampleRate());
  Serial.println(" uT");
  Serial.println();
  Serial.println("Magnetic Field in uT");
  Serial.println("X\tY\tZ");
}

void loop() {
  if (IMU.magneticFieldAvailable()) {
    float xMag, yMag, zMag;

    IMU.readMagneticField(xMag, yMag, zMag);

    Serial.print(xMag);
    Serial.print('\t');
    Serial.print(yMag);
    Serial.print('\t');
    Serial.println(zMag);

    if(zMag < 350) {
      showRedLight();
      Serial.println("Door opened");
    } else {
      turnOffLED();
      Serial.println("Door closed");
    }
  }
}

void initIMU() {
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
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

/*
  LED
*/
void showGreenLight() {
  digitalWrite(LEDR, HIGH);
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