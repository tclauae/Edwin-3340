#include <ESP8266WiFi.h>
#include <PubSubClient.h>
qqqqqqqqq
//#define ESP8266_LED 5   // GPIO 5 for NodeMCU, GPIO 0 for ESP8266 module
#define ESP8266_D1 5
#define ESP8266_D2 4
#define ESP8266_D3 D3
#define LED_HB D0

const char* topic_cmd = "/demo_room/kitchen01/light01/cmd";
const char* topic_state = "/demo_room/kitchen01/light01/state";

/* network dependent config */ 
const char* ssid = "CT-Staff";
const char* password =  "Computime#2010";

const char* mqttServer = "10.1.100.100";

const int mqttPort = 1883;
const char* mqttUser = "mqtt_client";
const char* mqttPassword = "YourMqttUserPassword";
 
WiFiClient espClient;
PubSubClient client(espClient);
int user = 0;

void relay_test(int count)
{
  for(int i=0; i<count;i++)
  {
    digitalWrite(ESP8266_D1, HIGH);
    delay(300);
    digitalWrite(ESP8266_D1, LOW);
    delay(300);
  }
}

void setup() {

  pinMode(LED_HB, OUTPUT);
  pinMode(ESP8266_D1, OUTPUT);
  pinMode(ESP8266_D2, OUTPUT);
  pinMode(ESP8266_D3, OUTPUT);

  relay_test(3);
  
  digitalWrite(LED_HB, HIGH);
  digitalWrite(ESP8266_D1, LOW);
  digitalWrite(ESP8266_D2, LOW);
  digitalWrite(ESP8266_D3, LOW);
  Serial.begin(115200);
 
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

void switch_off()
{
    delay(800);
    digitalWrite(ESP8266_D1, LOW);
    digitalWrite(ESP8266_D2, LOW);
    digitalWrite(ESP8266_D3, LOW);
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  
  Serial.println("Something received.");
  String string_payload = String((char*)payload);
  
  if(strncmp((char*)payload,"01",2) == 0)
  {
    Serial.println("Memory 01 is pressed.");
    if(user != 1){
      user = 1;
      digitalWrite(ESP8266_D1, HIGH);
      digitalWrite(ESP8266_D2, LOW);
      digitalWrite(ESP8266_D3, LOW);  
      switch_off();
      client.publish(topic_state, "01", true);  // true for retained message
    }
  } else if (strncmp((char*)payload,"02",2) == 0)
  {
    Serial.println("Memory 02 is pressed.");
    if(user != 2){
      user = 2;
      digitalWrite(ESP8266_D1, LOW);
      digitalWrite(ESP8266_D2, HIGH);
      digitalWrite(ESP8266_D3, LOW);
      switch_off();
      client.publish(topic_state, "02", true);
    }
  } else if (strncmp((char*)payload,"03",2) == 0)
  {
    Serial.println("Memory 03 is pressed.");
    if(user != 3){
      user = 3;
      digitalWrite(ESP8266_D1, LOW);
      digitalWrite(ESP8266_D2, LOW);
      digitalWrite(ESP8266_D3, HIGH);
      switch_off();
      client.publish(topic_state, "03", true);
    }
  } else if (strncmp((char*)payload,"OFF",3) == 0)
  {
    user = 0;
    Serial.println("Switch OFF");
    digitalWrite(ESP8266_D1, LOW);
    digitalWrite(ESP8266_D2, LOW);
    digitalWrite(ESP8266_D3, LOW);
    client.publish(topic_state, "OFF", true);
  }

}

int counter = 0;
void loop() {

  //Serial.println(counter);
  counter++;
  if(counter > 20000)
  {
    digitalWrite(LED_HB, HIGH);
  }
  if(counter > 40000)
  {
    digitalWrite(LED_HB, LOW);
    counter = 0;
  }
  //digitalWrite(LED_HB, HIGH);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
