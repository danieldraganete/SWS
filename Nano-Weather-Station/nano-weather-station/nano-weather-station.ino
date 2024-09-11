#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// MQTT configuration
const char* mqtt_server = "test.mosquitto.org"; // MQTT server address
const int mqtt_port = 1883; // MQTT server port
const char* mqtt_topic = "DDR-AMIN/weather/station"; // Topic for sending weather data
const char* ping_topic = "DDR-AMIN/weather/ping";  // Topic for ping messages

// WiFi configuration
const char* ssid = "Telia-EC2DEF";  // Your WiFi SSID
const char* password = "3DBF9327B5";  // Your WiFi password

WiFiClient espClient; // WiFi client instance
PubSubClient client(espClient); // MQTT client instance

// DHT sensor configuration
#define DHTPIN 2  // Pin where the DHT sensor is connected
#define DHTTYPE DHT11  // Type of the DHT sensor (DHT11)
DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor

// Rain Drop Sensor configuration
#define RAIN_SENSOR_ANALOG_PIN A0  // Analog pin for the Rain Drop sensor
#define RAIN_SENSOR_DIGITAL_PIN 2  // Digital pin for the Rain Drop sensor

// Light Sensor configuration
#define LIGHT_SENSOR_ANALOG_PIN A2  // Analog pin for the Light sensor

// Station identifier
const String station_id = "Wifi-MQTT";  // Unique ID for each station

void setup() {
    Serial.begin(115200); // Initialize serial communication at 115200 baud rate
    pinMode(RAIN_SENSOR_DIGITAL_PIN, OUTPUT);  // Set the Rain sensor digital pin as output
    
    // Initialize DHT sensor
    dht.begin();

    // Connect to WiFi
    connectWiFi();
    
    client.setServer(mqtt_server, mqtt_port); // Set MQTT server and port
    client.setCallback(callback); // Set callback function for incoming MQTT messages
    connectMQTT(); // Connect to MQTT server
}

void connectWiFi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password); // Start connecting to WiFi with provided credentials
    
    int attempt = 0; // Variable to keep track of connection attempts
    
    // Try to connect to WiFi until successful or attempts exceed 20
    while (WiFi.status() != WL_CONNECTED && attempt < 20) {
        delay(500);
        Serial.print(".");
        attempt++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP()); // Print local IP address
    } else {
        Serial.println("Failed to connect to WiFi");
        while (true); // Infinite loop if WiFi connection fails
    }
}

void connectMQTT() {
    // Attempt to connect to the MQTT server
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (client.connect("ESP32WeatherStation")) { // Try to connect with a client ID
            Serial.println("MQTT connected");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            delay(5000); // Wait 5 seconds before retrying
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    // Do nothing on receiving messages
}

void loop() {
    // Reconnect to MQTT if disconnected
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop(); // Keep the MQTT client loop active

    // Read data from DHT sensor
    float temperature = dht.readTemperature(); // Read temperature
    float humidity = dht.readHumidity(); // Read humidity
    String raining_status = ""; // To store rain status
    String light_condition = ""; // To store light condition

    // Logic to determine rain status based on sensor value
    if(analogRead(RAIN_SENSOR_ANALOG_PIN) < 300) raining_status = "Heavy Rain";
    else if(analogRead(RAIN_SENSOR_ANALOG_PIN) < 1000) raining_status = "Moderate Rain";
    else raining_status = "No Rain";

    // Logic to determine light condition based on sensor value
    if(analogRead(LIGHT_SENSOR_ANALOG_PIN) > 1000) light_condition = "Sunny";
    else if(analogRead(LIGHT_SENSOR_ANALOG_PIN) > 300) light_condition = "Cloudy";
    else light_condition = "Dark";

    // Check if the sensor data is valid
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Error reading sensor data!");

        // Send an error message to MQTT
        StaticJsonDocument<200> errorDoc;
        errorDoc["station_id"] = station_id;
        errorDoc["error_type"] = "Sensor failure";
        errorDoc["error_message"] = "Failed to read sensor data.";
        errorDoc["last_update"] = millis() / 1000;; // Time since the program started
        String errorPayload;
        serializeJson(errorDoc, errorPayload); // Serialize JSON data
        client.publish(mqtt_topic, errorPayload.c_str()); // Publish error message
        delay(10000); // Wait 10 seconds before the next measurement
        return;
    }

    // Display the sensor data on the Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Rain status : ");
    Serial.print(raining_status);
    Serial.print(", Analog Value : ");
    Serial.println(analogRead(RAIN_SENSOR_ANALOG_PIN));

    Serial.print("Light status : ");
    Serial.println(analogRead(LIGHT_SENSOR_ANALOG_PIN));
    
    // Create a JSON object for the sensor data
    StaticJsonDocument<200> dataDoc;
    dataDoc["station_id"] = station_id;
    dataDoc["temperature"] = temperature;
    dataDoc["humidity"] = humidity;
    dataDoc["light"] = light_condition;
    dataDoc["rain"] = raining_status; 
    dataDoc["last_update"] = millis() / 1000;;
    String payload;
    serializeJson(dataDoc, payload); // Serialize JSON data

    // Publish the sensor data to the MQTT topic
    if (client.publish(mqtt_topic, payload.c_str())) {
        Serial.println("/n Data published to MQTT");
    } else {
        Serial.println("Failed to publish data");
    }

    // Send a "pong" message to confirm the station is online
    StaticJsonDocument<200> pongDoc;
    pongDoc["station_id"] = station_id;
    pongDoc["pong"] = true;
    pongDoc["last_update"] = millis() / 1000;
    String pongPayload;
    serializeJson(pongDoc, pongPayload); // Serialize JSON data
    client.publish(ping_topic, pongPayload.c_str());

    delay(10000); // Wait 10 seconds before the next measurement
}
