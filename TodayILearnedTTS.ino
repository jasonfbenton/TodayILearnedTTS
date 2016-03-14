#include <ESP8266WiFi.h>;
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include "Emic2TtsModule.h"

#define rxPin   D9  // Serial input (connects to Emic 2's SOUT pin)
#define txPin   GPIO2  // Serial output (connects to Emic 2's SIN pin)


const char* ssid     = "twiot";
const char* password = "tworker01";

const char* mqtt_server = "atliot.com";

const char* host = "www.dannielle.me";

String tts;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String get_tts() {
  String tts_string;
  
  Serial.println("\nLet's browse Reddit!");

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    exit ;
  }
  
  // We now create a URI for the request
  String url = "/helloworld/index.html";
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(500);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    if (line.indexOf("Content-Type:") != -1) {
         tts_string = client.readString();
         // Remove leading /n
         tts_string.remove(0, 3);
    }
  }
  Serial.println();
  Serial.println(tts_string);
  Serial.println("closing connection"); 

  return tts_string;  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Send text to Serial1 if movement is detected in the Kitchen
  if ((char)payload[33] == 'M') {
    //digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    Serial1.print('S');
    delay(500);
    Serial1.print(tts);
    Serial.println(tts);
    // delay(5000);
    } 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("/motion");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  // Serial1 to Emic 2
  Serial1.begin(9600);
  delay(3000);
  Serial1.print('V');
  Serial1.print(18);
  Serial1.print('\n');
  Serial1.print('N');
  Serial1.print(1);
  Serial1.print('\n');

  // Connect to wifi
  setup_wifi();

  // Get Text to Speech string from web
  tts = get_tts();
  Serial.println(tts);
  // Connect to MQTT Server and subscribe to topic
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  
  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Subsrcribe to the motion topic on the MQTT Broker
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    client.subscribe("/motion");
  }

}
