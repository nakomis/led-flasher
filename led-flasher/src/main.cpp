#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

#define LOCAL_MQTT

#ifdef LOCAL_MQTT
    WiFiClient net = WiFiClient();
#else
    WiFiClientSecure net = WiFiClientSecure();
#endif

MQTTClient client = MQTTClient(256);

void messageHandler(String& topic, String& payload) {
    Serial.println("In MessageHandler");
    Serial.println("incoming: " + topic + " - " + payload);
    //  StaticJsonDocument<200> doc;
    //  deserializeJson(doc, payload);
    //  const char* message = doc["message"];
}
void connectAWS() {
    // Wait for the serial port to become available
    delay(5000);
    Serial.println("Hello, World!");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
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
    Serial.print("Connecting to AWS IOT");
#ifdef LOCAL_MQTT
    while (!client.connect(THINGNAME, LOCAL_MQTT_USERNAME, LOCAL_MQTT_PASSWORD)) {
#else
    while (!client.connect(THINGNAME)) {
#endif
        Serial.print(".");
        delay(100);
    }
    if (!client.connected()) {
        Serial.println("AWS IoT Timeout!");
        return;
    }
    // Subscribe to a topic
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
    Serial.println("AWS IoT Connected!");
}
void publishMessage() {
    StaticJsonDocument<200> doc;
    doc["time"] = millis();
    doc["sensor_a0"] = analogRead(0);
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer); // print to client
    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
void setup() {
    Serial.begin(9600);
    Serial.println("In setup!");
    connectAWS();
    delay(5000);
    Serial.println("Exiting Setup!");
}
void loop() {
    // publishMessage();
    client.loop();
    delay(1000);
}