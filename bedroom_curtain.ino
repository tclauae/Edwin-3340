#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>


#define ESP8266_D2 4
#define ESP8266_D4 2
#define EEPROM_SIZE 1
int state=0;


const char* topic_cmd = "/demo_room/bedroom01/curtain01/cmd";
const char* topic_state = "/demo_room/bedroom01/curtain01/state";
const char* topic_availability = "/demo_room/bedroom01/curtain01/availability";
/* network dependent config */ 
//const char* ssid = "CT-Staff";
//const char* password =  "Computime#2010";
const char* ssid = "TP-Link_83BB";
const char* password =  "22600300";


const char* mqttServer = "10.1.100.100";

const int mqttPort = 1883;
const char* mqttUser = "mqtt_client";
const char* mqttPassword = "YourMqttUserPassword";
 
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

  pinMode(ESP8266_D2, OUTPUT);
  pinMode(ESP8266_D4, OUTPUT);
  
  digitalWrite(ESP8266_D2, LOW);
  digitalWrite(ESP8266_D4, LOW);
  
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);

  state=EEPROM.read(0);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    if(client.connect(clientId.c_str())) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  client.subscribe(topic_cmd);
  client.publish(topic_availability, "available", true);

  //OTA
  {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      client.subscribe(topic_cmd);
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
  

void callback(char* topic, byte* payload, unsigned int length) {
  
  String string_payload = String((char*)payload);
  
  if(state != 1 &&  (strncmp((char*)payload,"OPEN",4) == 0))
  { state = 1;
    {
      digitalWrite(ESP8266_D2, HIGH);
      client.publish(topic_state, "OPEN", true);
      client.publish(topic_availability, "notavailable", true);
      delay(15000);
      digitalWrite(ESP8266_D2, LOW);
      client.publish(topic_availability, "available", true);
    }
  } else if (state != 2 && (strncmp((char*)payload,"CLOSE",5) == 0))
  { state = 2;
      digitalWrite(ESP8266_D4, HIGH);
      client.publish(topic_state, "CLOSE", true);
      client.publish(topic_availability, "notavailable", true);
      delay(15000);
      digitalWrite(ESP8266_D4, LOW);
      client.publish(topic_availability, "available", true);
  }
}


void loop() {

  ArduinoOTA.handle();

  if (!client.connected()) {
    reconnect();
  }
  if (state != state)
  {EEPROM.write(0,state);
   EEPROM.commit();}
  client.loop(); }
