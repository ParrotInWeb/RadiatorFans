#include <OneWire.h>
#include <DallasTemperature.h>

// --------------------- Electric relay settings ---------------------
const byte RELAY_PIN = 7;
bool relayState = false;

// --------------------- Temperature settings ---------------------
// Data wire is plugged into port 4 on the Arduino
#define ONE_WIRE_BUS 4
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// sensors addresses
const uint8_t sensorOnBoardAddress[8] = { 0x28, 0x3D, 0xD7, 0x96, 0xF0, 0x01, 0x3C, 0x0A };
const uint8_t sensorOnRadiatorAddress[8] = { 0x28, 0xFF, 0x89, 0x29, 0xA4, 0x16, 0x05, 0x82 };

// temparetures
float temperatureOnBoard;
float temperatureOnRadiator;
float temperatureDifference;

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

// --------------------- Fan settings ---------------------
// PWN is plugged into port 9 on the Arduino
const byte PWM_PIN = 9;

//Adjust this value to adjust the frequency (Frequency in HZ!) (Set currently to 25kHZ)
const word PWM_FREQ_HZ = 25000; 
const word TCNT1_TOP = 16000000 / (2 * PWM_FREQ_HZ);

int pwmDuty;

// temperatureLevels
const int tempearutreShutdown = 5;
const int tempearutreIntermediate = 20;
const int delayTime = 5000;

void setup(void) {
  Serial.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  sensors.begin();

  // Clear Timer1 control and count registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  TCCR1A |= (1 << COM1A1) | (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << CS10);
  ICR1 = TCNT1_TOP;
}

void loop(void) { 
  sensors.requestTemperatures(); // Send the command to get temperatures
  temperatureOnBoard = sensors.getTempC(sensorOnBoardAddress);
  temperatureOnRadiator = sensors.getTempC(sensorOnRadiatorAddress);
  temperatureDifference = temperatureOnRadiator - temperatureOnBoard;

  Serial.println("Sensor on board: " + String(temperatureOnBoard) + " ºC");
  Serial.println("Sensor on radiator: " + String(temperatureOnRadiator) + " ºC");
  
  relayState = (temperatureDifference > tempearutreShutdown) ? true : false;
  digitalWrite(RELAY_PIN, relayState);
  
  pwmDuty = (temperatureDifference < tempearutreShutdown) ? 0 : 
            (temperatureDifference < tempearutreIntermediate) ? 60 : 100;
  Serial.println("Set PWM duty: " + String(pwmDuty));
  setPwmDuty(pwmDuty);
  
  delay(delayTime);
}

void setPwmDuty(byte duty) {
  OCR1A = (word) (duty * TCNT1_TOP) / 100;
}

// function to get a device address
String getDeviceAddress(DeviceAddress deviceAddress) {
  String partOfAddress;
  String sensorAddress;
  for (uint8_t i = 0; i < 8; i++) {
    partOfAddress = String(deviceAddress[i], HEX);
    sensorAddress = sensorAddress + (partOfAddress.length() == 1 ? "0" + partOfAddress : partOfAddress);
  }
  if (sensorAddress.length() != 16) {
    sensorAddress = "Incorrect sensor address";
  } 
  return sensorAddress;
}
