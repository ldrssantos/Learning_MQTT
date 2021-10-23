// ----------------------------------------------------------------------------------
// -- Company: 
// -- Engineer:       Leandro Santos
// -- 
// -- Create Date:    10:56:33 10/22/2021 
// -- Project Name:   Grafana Dashboard
// -- Target Devices: ESP32 and compatible 
// -- Tool versions: 
// -- Description: Technical analysis for MQTT protocol
// --
// -- Revision: 
// -- Revision 0.01 - File Created
// -- Additional Comments: 
// --
// ----------------------------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>

#define MSG_BUFFER_SIZE (50)

// LED inteface settings
#define LED_CTRL_PIN  13
#define SYSTEM_LED_PIN 19

// WiFi interface connection
// const char* ssid = "your-ssid";
// const char* password =  "your-wifi-password"; 

// External MQTT Broker configuration
const char* mqtt_broker = "broker.mqttdashboard.com";
const char* mqtt_username = "unisal";
const char* mqtt_password = "unisal";
const int mqtt_port = 1883;

// MQTT defautl topics for this project
char clientId[MSG_BUFFER_SIZE];
const char* in_topic = "test_topic/led_ctrl";
char main_topic[MSG_BUFFER_SIZE] = "unisal_devices/";

// Additional variables and each initialization
char* out_topic;
char msg[MSG_BUFFER_SIZE];
int value = 0;
unsigned long lastMsg = 0;
int system_Led_status = false;

// WifiClinet and PubSubClient(over wifi connection) definition classes 
WiFiClient espClient;
PubSubClient client(espClient);

// Wifi Setup function
void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Internet connection (WiFi-ssid):  ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT callback defualt function 
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
    digitalWrite(LED_CTRL_PIN, LOW);   
    Serial.println("*** LED(OFF) ****");
  } else {
    digitalWrite(LED_CTRL_PIN, HIGH);  
    Serial.println("*** LED(ON) ****");
  }
}

// MQTT Server connection function
void reconnect() {
  uint64_t value = ESP.getEfuseMac();
  uint32_t MSB_part = value >> 32;
  uint32_t LSB_part = value & 0x00000000FFFFFFFF;
  
  snprintf(clientId, MSG_BUFFER_SIZE, "%04X%08X", MSB_part, LSB_part);
  out_topic = strcat(main_topic, clientId);
  
  Serial.println("**** out_topic = " + (String)out_topic + " ****");
 
  // MQTT login and Server connection while loop
  while (!client.connected()) {
    if (client.connect(clientId, mqtt_username, mqtt_password)) {
      Serial.println("**** MQTT Broker successfully connected ****");
      client.subscribe(in_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


// ********* ARDUINO DEFAULT DESIGN CODE STRUCTURE *********
void setup() {
  Serial.begin(115200);

  pinMode(LED_CTRL_PIN, OUTPUT);     
  pinMode(SYSTEM_LED_PIN, OUTPUT);     
 
  setup_wifi();
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  // MQTT connection verification 
  if (!client.connected()) {
    reconnect();
  }
  
  // MQTT callback buffer read
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    // snprintf (msg, MSG_BUFFER_SIZE, "hello world #%d", value);
    snprintf (msg, MSG_BUFFER_SIZE, "%d", value);
    
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(out_topic, msg);
    
    system_Led_status = !system_Led_status;
    digitalWrite(SYSTEM_LED_PIN, system_Led_status);
  }
}
