#include <MKRWAN.h>
#include <DHT.h>
#include <Wire.h>

// LoRa configuration
LoRaModem modem;
String appEui = "0000000000000000";  // Replace with your Application EUI
String appKey = "1EEF776A1E3C4951384BF7B834BB4BDD";  // Replace with your Application Key

// DHT11 sensor configuration
#define DHTPIN 3  // Pin where the DHT11 is connected
#define DHTTYPE DHT11  // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);

// Rain Drop Sensor configuration
#define RAIN_SENSOR_ANALOG_PIN A2  // Analog pin for the Rain Drop sensor
#define RAIN_SENSOR_DIGITAL_PIN 2  // Digital pin for the Rain Drop sensor

// Light Sensor configuration
#define LIGHT_SENSOR_PIN A1  // Analog pin for the Light sensor

// Port numbers
#define DATA_FPORT 2

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  while (!Serial);  // Wait for the serial port to be available

  // Initialize LoRa modem
  if (!modem.begin(EU868)) {  // Specify the frequency band for Europe
    Serial.println("Failed to start LoRa modem");
    while (1);  // Stay in a loop if initialization fails
  }

  // Print modem version and device EUI for verification
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  // Connect to LoRa network using OTAA (Over-The-Air Activation)
  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Failed to join LoRa network. Please check your connection.");
    while (1);  // Stay in a loop if joining the network fails
  } else {
    Serial.println("Joined LoRa network");
  }

  // Set LoRaWAN parameters
  modem.minPollInterval(60);  // Minimum interval between transmissions

  // Initialize DHT11 sensor
  dht.begin();

  // Configure pins for the Rain Drop sensor
  pinMode(RAIN_SENSOR_DIGITAL_PIN, OUTPUT);  // Set the Rain sensor digital pin as input
}

void loop() {
  // Read data from the DHT11 sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read data from the Rain Drop Sensor (analog and digital)
  int rainAnalogValue = analogRead(RAIN_SENSOR_ANALOG_PIN);
  int rainDigitalValue = digitalRead(RAIN_SENSOR_DIGITAL_PIN);

  // Read data from the Light Sensor
  int lightValue = analogRead(LIGHT_SENSOR_PIN);

  // Determine the rain status based on analog reading
  String rainStatus;
  if (rainAnalogValue < 300) {
    rainStatus = "Heavy Rain";
  } else if (rainAnalogValue < 500) {
    rainStatus = "Moderate Rain";
  } else {
    rainStatus = "No Rain";
  }

  // Determine the light status based on the light sensor reading
  String lightStatus;
  if (lightValue > 1000) {
    lightStatus = "Sunny"; 
  } else if (lightValue > 500) {
    lightStatus = "Cloudy";  
  } else {
    lightStatus = "Dark";  
  }

  // Check for errors from the DHT11 sensor
  bool temperatureAvailable = !isnan(temperature);
  bool humidityAvailable = !isnan(humidity);
  int errorCode = 0;

  if (!temperatureAvailable) {
    errorCode = 1;  // Example error code for temperature
  }

  if (!humidityAvailable) {
    errorCode = 2;  // Example error code for humidity
  }

  // Prepare the payloads
  String dataPayload = String("Tmp: ") + String(temperature, 2) +
                       " Hum: " + String((int)humidity) +
                       " Light: " + lightStatus +
                       " Rain: " + rainStatus;
  
  // Print the payloads to the serial monitor for debugging
  Serial.print("Sending data payload: ");
  Serial.println(dataPayload);
  Serial.println(lightValue);
  sendLoRaPacket(dataPayload, DATA_FPORT);

  // Wait before the next reading
  delay(60000);  // Send data every 60 seconds
}

void sendLoRaPacket(String payload, int fPort) {
  int err;
  modem.beginPacket();
  modem.write(fPort);  // Set the FPort
  modem.print(payload);
  err = modem.endPacket();  // Removed the 'true' to avoid requiring an ACK

  if (err > 0) {
    Serial.println("Data sent successfully.");
  } else {
    Serial.print("Failed to send data. Error code: ");
    Serial.println(err);
    Serial.print("Payload: ");
    Serial.println(payload);
  }
}

