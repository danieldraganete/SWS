# Weather Station System

## Design and Implementation

### Introduction
This report details the design and implementation of a comprehensive weather monitoring system. The system features distributed weather stations and a central station for data aggregation and visualization. Utilizing Arduino Nano ESP32 and Arduino MKR WAN 1300 for data collection, the system employs MQTT and LoRaWAN for data transmission. The central station, equipped with an ESP32-2432S028, is responsible for real-time data display. This document encompasses the system architecture, component descriptions, implementation details, and documentation, including 3D printed enclosures, mounting, and installation instructions.

### System Components

#### Weather Stations

- **Arduino Nano ESP32**
  - **Sensors:** Temperature, humidity, light, rain
  - **Communication:** Data transmission via MQTT (broker: test.mosquitto.org)

- **Arduino MKR WAN 1300**
  - **Sensors:** Similar to Arduino Nano ESP32
  - **Communication:** Data transmission via LoRaWAN to TTN (The Things Network)

#### Main Station

- **ESP32-2432S028**
  - **Processor:** ESP-WROOM-32
  - **Display:** 2.8-inch TFT touchscreen
  - **Functionality:** Aggregates and displays data from weather stations

### System Architecture

#### Weather Stations

- **Arduino Nano ESP32**
  - **Sensors:**
    - DHT11: Temperature and humidity
    - Rain Sensor: Detects precipitation
    - Light Sensor: Measures ambient light
  - **Communication:** Publishes data to the MQTT broker. Any station connected to this broker can access data from other stations via the same topic.

- **Arduino MKR WAN 1300**
  - **Sensors:** Similar to Arduino Nano ESP32
  - **Communication:** Sends data via LoRaWAN to TTN. A server API redirects data from TTN to the MQTT broker.

#### Main Station

- **ESP32-2432S028**
  - **Display:** TFT screen for data presentation
  - **Functions:**
    - **WiFi Connectivity:** Connects to the internet and the MQTT broker
    - **Data Aggregation:** Receives and processes data from weather stations
    - **Data Display:** Shows real-time data on the touchscreen

### Implementation Details

#### Data Transmission by Stations

- **Arduino Nano ESP32:**
  - **Code:** Collects sensor data and publishes it to the MQTT broker.
  - **Design:** Data is transmitted to a specified topic, allowing any station connected to the broker to access data from other stations.

- **Arduino MKR WAN 1300:**
  - **Code:** Sends data via LoRaWAN to TTN.
  - **Data Redirection:** A server API redirects data from TTN to the MQTT broker.

#### Main Station Code

The main code for the ESP32 includes:

1. **Initialization:**
   - Setting up the display and establishing WiFi and MQTT connections.
   - Displays a welcome message while connecting to WiFi.

2. **WiFi Connectivity:**
   - Connects to the specified WiFi network and displays the IP address.

3. **MQTT Connectivity:**
   - Connects to the MQTT broker and subscribes to the relevant topic.
   - Displays the connection status on the screen.

4. **Data Handling:**
   - **MQTT Callback:** Processes incoming MQTT messages and updates station data.
   - **API Data Handling:** Receives data redirected from TTN via the MQTT broker.
   - **Display Update:** Shows data on the touchscreen.

5. **Display Functionality:**
   - **Data Display:** Presents data for the current station.
   - **Navigation:** Allows switching between different stations.

6. **Loop Function:**
   - Monitors and maintains MQTT connection.
   - Updates the display with new data.

### 3D Printed Enclosures

- **Main Station Enclosure:** Designed to house the ESP32 and related components.
- **Weather Station Enclosures:** Custom enclosures for sensors and Arduino boards.

3D printing files are included in the report folder.

### Mounting and Installation Instructions

1. **Mounting Weather Stations:**
   - Install sensors and Arduino boards in a protected enclosure.
   - Connect the station to the WiFi network (for Arduino Nano ESP32) or LoRaWAN network (for Arduino MKR WAN 1300).

2. **Mounting the Main Station:**
   - Assemble the ESP32 and display in a suitable enclosure.
   - Set up WiFi and MQTT connections.
   - Ensure access to data from weather stations.

### Conclusion
The weather station system provides a robust solution for real-time environmental monitoring. The integration of sensors, communication technologies, and display functionalities offers a powerful tool for weather data analysis and visualization.

### Future Work

- **Improvements:** Adding additional sensors or alternative communication methods.
- **Data Analysis:** Implementing advanced data processing and visualization techniques.
- **User Interface:** Enhancing interaction and navigation on the display.

The provided report and code offer a comprehensive overview of the weather station system design and functionality. Additional documentation, including 3D printing files and component datasheets, is included for a complete understanding and implementation.
