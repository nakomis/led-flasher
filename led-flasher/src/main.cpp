#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <vector>

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

const int LED_RED = 2;
const int LED_GREEN = 2;
const int LED_BLUE = 2;
const int LED_BUILTIN = 2;
const int FLASH_COUNT = 5;
const int FLASH_DURATION = 500;

// Uncomment the following line to target a local MQTT
// #define LOCAL_MQTT

#ifdef LOCAL_MQTT
    WiFiClient net = WiFiClient();
#else
    WiFiClientSecure net = WiFiClientSecure();
#endif

MQTTClient client = MQTTClient(256);

int colourToLed(String colour) {
    if (colour == "RED") {
        return LED_RED;
    } else if (colour == "GREEN") {
        return LED_GREEN;
    } else {
        return LED_BLUE;
    }
}

void on(std::vector<String> leds) {
    for (int i = 0; i < leds.size(); i++) {
        int led = colourToLed(leds[i]);
        Serial.println("ON " + leds[i] + " on pin " + String(led));
        digitalWrite(led, LOW);
    }
}

void off(std::vector<String> leds) {
    for (int i = 0; i < leds.size(); i++) {
        int led = colourToLed(leds[i]);
        Serial.println("OFF " + leds[i] + " on pin " + String(led));
        digitalWrite(led, HIGH);
    }
}

void flash(std::vector<String> leds) {
    for (int count = 0; count < FLASH_COUNT; count++) {
        on(leds);
        delay(FLASH_DURATION);
        off(leds);
        delay(FLASH_DURATION);
    }
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

    JsonDocument doc;
    deserializeJson(doc, payload);

    String message = doc["message"];
    std::vector<String> words = splitStringToVector(message);
    String action = words[0];
    String led = words[1];
    std::vector<String> leds;
    if (led == "ALL") {
        leds = {"RED", "GREED", "BLUE"};
    } else {
        leds = {led};
    }

    if (action == "FLASH") {
        flash(leds);
    }

    if (action == "ON") {
        on(leds);
    }

    if (action == "OFF") {
        off(leds);
    }

    Serial.println("Outputting words...: ");
    for (int i = 0; i < words.size(); i++) {
        Serial.println("Word " + String(i) + ": " + words[i]);
    }
    Serial.println();
}

void connectAWS() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi.");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
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

#ifdef LOCAL_MQTT
    while (!client.connect(THINGNAME, LOCAL_MQTT_USERNAME, LOCAL_MQTT_PASSWORD)) {
#else
    while (!client.connect(THINGNAME)) {
#endif
        Serial.print(".");
        delay(100);
    }
    Serial.println("");

    if (!client.connected()) {
        Serial.println("AWS IoT Timeout!");
        return;
    }
    // Subscribe to a topic
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
    Serial.println("AWS IoT Connected.\n");
}

void publishMessage() {
    JsonDocument doc;
    doc["time"] = millis();
    doc["sensor_a0"] = analogRead(0);
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);
    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void setup() {
    Serial.begin(9600);
    // Wait for the serial port to become available
    delay(5000);
    Serial.println("\n\n");
    Serial.println("======================");
    Serial.println("=== Starting Setup ===");
    Serial.println("======================\n");
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    connectAWS();
    Serial.println("======================");
    Serial.println("=== Setup Complete ===");
    Serial.println("======================\n");
}

void loop() {
    // publishMessage();
    client.loop();
    delay(1000);
}