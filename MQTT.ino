#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <string>
// Wifi 
const char *ssid = "Sebastian ";
const char *password = "191892526";

// Broker
const char *mqtt_broker = "192.168.100.8";
const char *topic = "ESP8266";
const int port = 1883;

// DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
int timeSinceLastRead = 0;

// Data
String temp_str; //see last code block below use these to convert the float that you get back from DHT to a string =str
String hum_str;
char temp[50];
char hum[50];

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print('.');
  }

  Serial.println("");
  Serial.println("Conectado a WiFi");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("", "", "")) {
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

float temperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Error reading temperature");
    return -999.999;  
  }
  return t;
}

float humidity() {
  float h = dht.readHumidity();
  if(isnan(h)) {
    Serial.println("Error reading humidity");
    return -999.999;
  }
  return h;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(2000);
  pinMode(5, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_broker  , port);
  dht.begin();
  Serial.println("Device started");
  Serial.println("----------------------------");
  Serial.println("Running DHT!");
  Serial.println("----------------------------");
  
}


void handleDHTMessage(float temperature, float humidity) {

  // Friendly log
  Serial.print("Temperature: ");
  Serial.print(temperature);  
  Serial.print(" Humidity: ");
  Serial.println(humidity );
  
  // Publish to broker
  client.publish("ESP8266/DHT22/TEMP", String(temperature).c_str());
  client.publish("ESP8266/DHT22/HUM", String(humidity).c_str());
}

  
int lastmsg = 0;
void refresh(int interval) {
  long now = millis();
  if (now - lastmsg > interval) {
    lastmsg = now;
    float t = temperature();
    float h = humidity();
    handleDHTMessage(t, h);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!client.connected()) {
    reconnect();
    
  }
  client.loop();
  refresh(5000);
}
