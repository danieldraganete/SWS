from flask import Flask, jsonify
import requests
import paho.mqtt.client as mqtt
import json
import time

app = Flask(__name__)

# MQTT configuration
MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT = 1883
MQTT_TOPIC = "DDR-AMIN/weather/station"
MQTT_CLIENT_ID = "API_Server_Client"

# TTN configuration
TTN_API_URL = "https://<your-ttn-server>/api/v3/as/applications/<application-id>/devices/<device-id>/packages/storage/uplink_message"
TTN_API_KEY = "<your-api-key>"

# Initialize MQTT client
mqtt_client = mqtt.Client(MQTT_CLIENT_ID)

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with result code {rc}")

mqtt_client.on_connect = on_connect
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
mqtt_client.loop_start()

@app.route('/fetch_and_forward', methods=['GET'])
def fetch_and_forward():
    # Fetch data from TTN using the specified request
    headers = {
        'Authorization': f'Bearer {TTN_API_KEY}'
    }
    response = requests.get(TTN_API_URL, headers=headers)
    if response.status_code == 200:
        data = response.json()
        # Extract payload and decode it
        payload = extract_latest_payload(data)
        decoded_data = decode_payload(payload)

        # Publish data to the MQTT broker
        mqtt_client.publish(MQTT_TOPIC, json.dumps(decoded_data), qos=0, retain=False)
        return jsonify({"message": "Data fetched from TTN and published to MQTT", "data": decoded_data}), 200
    else:
        return jsonify({"error": "Failed to fetch data from TTN"}), response.status_code

def extract_latest_payload(data):
    # Extract the latest message from the payload
    if 'result' in data and len(data['result']) > 0:
        latest_message = data['result'][0]  # Assume the first item is the most recent
        if 'uplink_message' in latest_message:
            # Extract the decoded payload
            return latest_message['uplink_message']['decoded_payload']  # Adjust based on exact message structure
    return ""

def decode_payload(payload):
    # Decode the TTN payload message
    data = {}
    try:
        # Decode the specified payload
        # Example payload: Tmp: 27.30 Hum: 42 Light: Cloudy Rain: No Rain
        payload_dict = {}
        parts = payload.split(' ')
        for i in range(0, len(parts), 2):
            key = parts[i].strip(':')
            value = parts[i + 1]
            if key == 'Tmp':
                payload_dict['temperature'] = float(value)
            elif key == 'Hum':
                payload_dict['humidity'] = float(value)
            elif key == 'Light':
                payload_dict['light'] = value
            elif key == 'Rain':
                payload_dict['rain'] = value
        
        # Add additional information
        data['station_id'] = "Wifi-MQTT"
        data['temperature'] = payload_dict.get('temperature', -1)
        data['humidity'] = payload_dict.get('humidity', -1)
        data['light'] = payload_dict.get('light', 'N/A')
        data['rain'] = payload_dict.get('rain', 'N/A')
        data['last_update'] = int(time.time())  # Add current timestamp
    except Exception as e:
        print(f"Error decoding payload: {e}")
        data = {"error": "Failed to decode payload"}
    return data

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
