#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <vector>
#include <map>

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

const String RED = "RED";
const String GREEN = "GREEN";
const String BLUE = "BLUE";
const String ON = "ON";
const String OFF = "OFF";
const String FLASH = "FLASH";

const int LED_RED = 13;
const int LED_GREEN = 12;
const int LED_BLUE = 14;
const int LED_BUILTIN = 2;
const int FLASH_COUNT = 5;
const int FLASH_DURATION = 200;
const int SEQUENCE_DURATION = 100;
const unsigned long MQTT_START_TIMEOUT_MILLIS = 20 * 1000;

const int RED_VALUE = LOW;

std::map<String, uint8_t> ledStates{{RED, 0}, {GREEN, 0}, {BLUE, 0}};

// Uncomment the following line to target a local MQTT
// #define LOCAL_MQTT

#ifdef LOCAL_MQTT
    WiFiClient net = WiFiClient();
#else
    WiFiClientSecure net = WiFiClientSecure();
#endif

MQTTClient client = MQTTClient(256);

int colourToLed(String colour) {
    if (colour.equals(RED)) {
        return LED_RED;
    } else if (colour.equals(GREEN)) {
        return LED_GREEN;
    } else {
        return LED_BLUE;
    }
}

void setPin(std::vector<String> leds, uint8_t value) {
    for (int i = 0; i < leds.size(); i++) {
        int led = colourToLed(leds[i]);
        digitalWrite(led, led == LED_BUILTIN ? !value : value); // LED_BUILTIN is reversed for some reason...
        ledStates[leds[i]] = value;
    }
}

void togglePin(String led) {
    setPin({led}, ledStates[led]);
    ledStates[led] = !ledStates[led];
}

void flash(std::vector<String> leds) {
    // Save the current state of the LEDs
    std::map<String, uint8_t> initialStates(ledStates);

    // Start with all target LEDs switched off
    setPin(leds, LOW);
    delay(FLASH_DURATION);

    // Flash the LEDs
    for (int count = 0; count < FLASH_COUNT; count++) {
        setPin(leds, HIGH);
        delay(FLASH_DURATION);
        setPin(leds, LOW);
        delay(FLASH_DURATION);
    }

    // Reset the LEDs to the initial states
    setPin({RED}, initialStates[RED]);
    setPin({GREEN}, initialStates[GREEN]);
    setPin({BLUE}, initialStates[BLUE]);
}

void sequence(int count = 1) {
    // Save the current state of the LEDs
    std::map<String, uint8_t> initialStates(ledStates);

    // Start with all LEDs switched off
    setPin({RED, GREEN, BLUE}, LOW);
    delay(FLASH_DURATION);

    for (int i = 0; i < count; i++) {
        for (const String colour : {RED, GREEN, BLUE}) {
            setPin({colour}, HIGH);
            delay(SEQUENCE_DURATION);
            setPin({colour}, LOW);
        }
    }

    delay(FLASH_DURATION);

    // Reset the LEDs to the initial states
    setPin({RED}, initialStates[RED]);
    setPin({GREEN}, initialStates[GREEN]);
    setPin({BLUE}, initialStates[BLUE]);
}

std::vector<String> splitStringToVector(String msg){
  std::vector<String> subStrings;
  int j=0;
  for(int i =0; i < msg.length(); i++){
    if(msg.charAt(i) == ' '){
      subStrings.push_back(msg.substring(j,i));
      j = i+1;
    }
  }
  subStrings.push_back(msg.substring(j,msg.length())); //to grab the last value of the string
  return subStrings;
}

void messageHandler(String& topic, String& payload) {
    Serial.println("Incoming: " + topic + " - " + payload);

    Serial.println("Payload: " + payload);

    JsonDocument doc;
    deserializeJson(doc, payload);

    String action = doc["action"];
    String led = doc["led"];

    std::vector<String> leds;    
    if (led.equals("ALL")) {
        leds = {RED, GREEN, BLUE};
    } else {
        leds = {led};
    }

    Serial.println("The action is: " + action);

    if (action.equals("FLASH")) {
        Serial.println("Flashing");
        flash(leds);
    }

    if (action.equals("ON")) {
        Serial.println("Oning");
        setPin(leds, HIGH);
    }

    if (action.equals("OFF")) {
        Serial.println("Offing");
        setPin(leds, LOW);
    }

    Serial.println();
}

void connectAWS() {
    setPin({RED, GREEN, BLUE}, LOW);
    setPin({RED}, HIGH);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi.");

    unsigned long startMillis = millis();

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startMillis > MQTT_START_TIMEOUT_MILLIS) {
            Serial.println();
            Serial.println("Bailing out of WiFi Connect");
            WiFi.disconnect();
            return;
        }
        togglePin(GREEN);
        delay(SEQUENCE_DURATION);
        Serial.print(".");
    }
    setPin({RED, GREEN}, HIGH);
    Serial.println("\nWi-Fi Connected.\n");
    client.setKeepAlive(31);

#ifdef LOCAL_MQTT
    // Connect to the MQTT broker on the AWS endpoint we defined earlier
    client.begin(LOCAL_MQTT_HOSTNAME, 8883, net);
#else
    // Configure WiFiClientSecure to use the AWS IoT device credentials
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);
    // Connect to the MQTT broker on the AWS endpoint we defined earlier
    client.begin(AWS_IOT_ENDPOINT, 8883, net);
#endif
    
    // Create a message handler
    client.onMessage(messageHandler);
    Serial.print("Connecting to AWS IOT.");

    startMillis = millis();

#ifdef LOCAL_MQTT
    while (!client.connect(THINGNAME, LOCAL_MQTT_USERNAME, LOCAL_MQTT_PASSWORD)) {
#else
    while (!client.connect(THINGNAME)) {
#endif
        if (millis() - startMillis > MQTT_START_TIMEOUT_MILLIS) {
            Serial.println();
            Serial.println("Bailing out of MQTT Connect");
            WiFi.disconnect();
            return;
        }
        Serial.print(".");
        togglePin(BLUE);
        delay(SEQUENCE_DURATION);
    }
    setPin({RED, GREEN, BLUE}, HIGH);
    Serial.println("");

    if (!client.connected()) {
        Serial.println("AWS IoT Timeout!");
        return;
    }
    // Subscribe to a topic
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
    Serial.println("AWS IoT Connected.\n");
    delay(1000);
    setPin({RED, GREEN, BLUE}, LOW);
}

void publishMessage() {
    JsonDocument doc;
    doc["time"] = millis();
    doc["sensor_a0"] = analogRead(0);
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);
    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

// For main() see https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/main.cpp

void setup() {
    const std::vector<int> pins = {LED_BUILTIN, LED_RED, LED_GREEN, LED_BLUE};
    for (int i = 0; i < pins.size(); i++) {
        int pin = pins[i];
        pinMode(pin, OUTPUT);
    }
    setPin({RED, GREEN, BLUE}, LOW);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(9600);
    // Wait for the serial port to become available
    delay(5000);
    
    Serial.println("\n\n");
    Serial.println("======================");
    Serial.println("=== Starting Setup ===");
    Serial.println("======================\n");
    connectAWS();
    Serial.println("======================");
    Serial.println("=== Setup Complete ===");
    Serial.println("======================\n");
    sequence(3);
}

void loop() {
    while (WiFi.status() != WL_CONNECTED || !client.connected()) {
        connectAWS();
    }
    client.loop();
    delay(1000);
}