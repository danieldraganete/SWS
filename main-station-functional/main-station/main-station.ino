#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32_SPI_9341.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


// MQTT Configuration
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "DDR-AMIN/weather/station";

// WiFi Configuration
const char* ssid = "Telia-EC2DEF";
const char* password = "3DBF9327B5";

WiFiClient espClient;
PubSubClient client(espClient);
LGFX lcd;  // Object for display

// NTP Configuration
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200, 60000); // UTC+1 for standard time (use 7200 for daylight saving time)

// Dimensions and margins for arrows
const int arrowSize = 20;  // Arrow size
const int arrowMargin = 10;  // Arrow margin from the edge of the screen

// Sensor data for different stations
struct SensorData {
    String station_id;
    float temperature;
    float humidity;
    String light;
    String rain;
    bool temperatureAvailable;
    bool humidityAvailable;
    bool lightAvailable;
    bool rainAvailable;
    bool hasError;
    String errorType;
    String errorMessage;
    bool isConnected;
    unsigned long lastUpdate;  // Added to check the last update
} stations[3];

int currentStation = 0;  // Currently displayed station

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.setRotation(3);
    lcd.fillScreen(TFT_BLACK);

    displayWelcome();
    connectWiFi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    connectMQTT();

    initializeStations();

    drawArrows();
    timeClient.begin();
}

void displayWelcome() {
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_CYAN, TFT_BLACK);
    lcd.setTextSize(3);
    lcd.setCursor(10, 10);
    lcd.println("Connecting to WiFi...");
}

void connectWiFi() {
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_CYAN, TFT_BLACK);
    lcd.setTextSize(2);
    lcd.setCursor(10, 10);
    lcd.printf("Connecting to %s...", ssid);

    WiFi.begin(ssid, password);
    int attempt = 0;

    while (WiFi.status() != WL_CONNECTED && attempt < 20) {
        delay(500);
        lcd.setCursor(10, 30);
        lcd.printf("Attempt %d...", attempt + 1);
        attempt++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        lcd.setCursor(10, 50);
        lcd.printf("Connected to %s", ssid);
        lcd.setCursor(10, 70);
        lcd.printf("IP Address: %s", WiFi.localIP().toString().c_str());
    } else {
        lcd.setCursor(10, 50);
        lcd.println("Failed to connect.");
        while (true);
    }
}

void connectMQTT() {
    while (!client.connected()) {
        lcd.fillScreen(TFT_BLACK);
        lcd.setTextColor(TFT_CYAN, TFT_BLACK);
        lcd.setCursor(10, 10);
        lcd.println("Connecting to MQTT...");
        if (client.connect("ESP32WeatherClient")) {
            lcd.setCursor(10, 30);
            lcd.println("MQTT connected");
            client.subscribe(mqtt_topic);
        } else {
            lcd.setCursor(10, 30);
            lcd.printf("MQTT failed, rc=%d", client.state());
            delay(5000);
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    String payloadStr = "";
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    StaticJsonDocument<400> doc;
    DeserializationError error = deserializeJson(doc, payloadStr);
    if (error) {
        return;
    }

    String station_id = doc["station_id"].as<String>();
    float temperature = doc["temperature"].as<float>();
    float humidity = doc["humidity"].as<float>();
    String light = doc["light"].as<String>();
    String rain = doc["rain"].as<String>();
    String errorType = doc.containsKey("error_type") ? doc["error_type"].as<String>() : "";
    String errorMessage = doc.containsKey("error_message") ? doc["error_message"].as<String>() : "";

    bool stationFound = false;
    for (int i = 0; i < 3; i++) {
        if (stations[i].station_id == station_id) {
            stations[i].temperature = temperature;
            stations[i].humidity = humidity;
            stations[i].light = light;
            stations[i].rain = rain;
            stations[i].temperatureAvailable = doc.containsKey("temperature");
            stations[i].humidityAvailable = doc.containsKey("humidity");
            stations[i].lightAvailable = doc.containsKey("light");
            stations[i].rainAvailable = doc.containsKey("rain");
            stations[i].hasError = !errorType.isEmpty();
            stations[i].errorType = errorType;
            stations[i].errorMessage = errorMessage;
            stations[i].lastUpdate = millis();
            stations[i].isConnected = true;
            stationFound = true;
            break;
        }
    }

    if (!stationFound) {
        for (int i = 0; i < 3; i++) {
            if (stations[i].station_id == "") {
                stations[i].station_id = station_id;
                stations[i].temperature = temperature;
                stations[i].humidity = humidity;
                stations[i].light = light;
                stations[i].rain = rain;
                stations[i].temperatureAvailable = doc.containsKey("temperature");
                stations[i].humidityAvailable = doc.containsKey("humidity");
                stations[i].lightAvailable = doc.containsKey("light");
                stations[i].rainAvailable = doc.containsKey("rain");
                stations[i].hasError = !errorType.isEmpty();
                stations[i].errorType = errorType;
                stations[i].errorMessage = errorMessage;
                stations[i].lastUpdate = millis();
                stations[i].isConnected = true;
                break;
            }
        }
    }

    displayData(currentStation);
}

void loop() {
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();

    timeClient.update();  // Update time

    // Debugging: Check the current time
    Serial.print("Hour: ");
    Serial.println(timeClient.getHours());
    Serial.print("Minute: ");
    Serial.println(timeClient.getMinutes());

    checkStationStatus();

    int x, y;
    if (lcd.getTouch(&x, &y)) {
        if (x < lcd.width() / 2 && y > lcd.height() - arrowSize) {
            currentStation = (currentStation - 1 + 3) % 3;
            displayData(currentStation);
        } else if (x > lcd.width() / 2 && y > lcd.height() - arrowSize) {
            currentStation = (currentStation + 1) % 3;
            displayData(currentStation);
        }
        delay(200);
    }
}

void displayData(int stationIndex) {
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_WHITE);
    lcd.setTextSize(2);

    lcd.setCursor(10, 10);
    lcd.printf("Station ID: %s", stations[stationIndex].station_id.c_str());

    lcd.setCursor(10, 30);
    if (stations[stationIndex].isConnected) {
        lcd.setTextColor(TFT_GREEN);
        lcd.printf("Status: Online");
    } else {
        lcd.setTextColor(TFT_RED);
        lcd.printf("Status: Offline");
    }

    lcd.setCursor(10, 50);
    lcd.setTextColor(TFT_WHITE);
    lcd.printf("Temperature: %.1f C", stations[stationIndex].temperatureAvailable ? stations[stationIndex].temperature : -1);

    lcd.setCursor(10, 70);
    lcd.printf("Humidity: %.1f%%", stations[stationIndex].humidityAvailable ? stations[stationIndex].humidity : -1);

    lcd.setCursor(10, 90);
    lcd.printf("Light: %s", stations[stationIndex].lightAvailable ? stations[stationIndex].light : "N/A");

    lcd.setCursor(10, 110);
    lcd.printf("Rain: %s", stations[stationIndex].rainAvailable ? stations[stationIndex].rain : "N/A");

    // Display date and time
    lcd.setTextSize(1);  // Smaller size for date and time
    lcd.setCursor(10, lcd.height() - 10);
    lcd.setTextColor(TFT_CYAN);
    lcd.printf("Time: %02d:%02d", timeClient.getHours(), timeClient.getMinutes());


    if (stations[stationIndex].hasError) {
        lcd.setCursor(10, 150);
        lcd.setTextColor(TFT_RED);
        lcd.printf("Error: %s - %s", stations[stationIndex].errorType.c_str(), stations[stationIndex].errorMessage.c_str());
    }
}

void checkStationStatus() {
    unsigned long currentMillis = millis();
    for (int i = 0; i < 3; i++) {
        if (stations[i].isConnected && (currentMillis - stations[i].lastUpdate > 60000)) { // 60 sec
            stations[i].isConnected = false;
            stations[i].hasError = true;
            stations[i].errorType = "Connection Lost";
            stations[i].errorMessage = "No data received for 60 seconds";
            displayData(i);
        }
    }
}

void drawArrows() {
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    lcd.setTextSize(3);
    
    // Left arrow
    lcd.fillTriangle(arrowMargin, lcd.height() - arrowSize, arrowMargin + arrowSize, lcd.height() - arrowSize / 2, arrowMargin, lcd.height() - arrowSize / 2);
    
    // Right arrow
    lcd.fillTriangle(lcd.width() - arrowMargin - arrowSize, lcd.height() - arrowSize, lcd.width() - arrowMargin, lcd.height() - arrowSize / 2, lcd.width() - arrowMargin - arrowSize, lcd.height() - arrowSize / 2);
}

void initializeStations() {
    for (int i = 0; i < 3; i++) {
        stations[i].station_id = "";
        stations[i].temperature = -1;
        stations[i].humidity = -1;
        stations[i].light = -1;
        stations[i].rain = false;
        stations[i].temperatureAvailable = false;
        stations[i].humidityAvailable = false;
        stations[i].lightAvailable = false;
        stations[i].rainAvailable = false;
        stations[i].hasError = false;
        stations[i].errorType = "";
        stations[i].errorMessage = "";
        stations[i].isConnected = false;
        stations[i].lastUpdate = 0;
    }
}
