# Pet Feeding IoT Project

This project is designed to automate pet feeding using an ESP8266 microcontroller, load cell (HX711), and a motor. The system is also integrated with MQTT communication for remote monitoring and control.

## Overview

The project consists of the following components:

- **ESP8266 Microcontroller**: Controls the overall operation of the system, including connecting to Wi-Fi, interfacing with the load cell, and controlling the motor.
- **Load Cell (HX711)**: Measures the weight of the pet food in the feeding container.
- **Motor**: Dispenses pet food based on predetermined settings.
- **MQTT Communication**: Facilitates communication with a remote server for registration, feeding commands, and status updates.

## Requirements

- Arduino IDE
- ESP8266WiFi library
- ArduinoJson library
- PubSubClient library
- WiFiClientSecure library
- WiFiManager library
- ESP_EEPROM library
- HX711_ADC library
- LiquidCrystal_I2C library

## Setup

1. **Hardware Setup**:
   - Connect the load cell to the ESP8266 microcontroller using the designated pins.
   - Connect the motor to the microcontroller for food dispensing.

2. **Software Setup**:
   - Install the required libraries in the Arduino IDE.
   - Upload the provided code to the ESP8266 microcontroller.

3. **Wi-Fi Configuration**:
   - Upon powering up, the device will create a Wi-Fi access point named "ESP8266".
   - Connect your computer or smartphone to this Wi-Fi network.
   - Open a web browser and navigate to the IP address displayed on the serial monitor.
   - Follow the instructions to configure the Wi-Fi settings of the device.

4. **MQTT Configuration**:
   - Modify the MQTT server information (server address, port, username, password) in the code according to your setup.

5. **Calibration**:
   - Calibrate the load cell using a known weight to ensure accurate measurements.
   - Adjust the calibration factor in the code based on the calibration results.

## Usage

1. **Startup**:
   - Power on the device.
   - It will connect to the configured Wi-Fi network and MQTT broker automatically.

2. **Registration**:
   - Upon successful Wi-Fi connection, the device will attempt to register with the MQTT server.
   - Once registered, it will receive a unique station ID from the server.

3. **Feeding**:
   - Send feeding commands to the device via MQTT.
   - The device will dispense the specified amount of pet food when instructed.

4. **Monitoring**:
   - Monitor the remaining food level and device status through MQTT messages.
   - The device periodically sends updates on the remaining food level and other relevant information.
[DEMO CLIP](https://youtu.be/VCjP-1S2tZ8)

## MQTT Topics

- `station/register/server/response`: Response from the server upon registration.
- `station/check/server`: Check command from the server.
- `feedRecord/start/server/send`: Command to start feeding from the server.
- `feedRecord/start/device/response`: Response from the device after receiving feeding instructions.
- `station/info`: Information about the device status and remaining food level.

## Contributors

- TrucLeK21
- hungvo2003vn

## License

This project is licensed under the [MIT License](LICENSE).
