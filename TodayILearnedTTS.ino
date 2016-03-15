#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include "Emic2TtsModule.h"



#define rxPin   D9  // Serial input (connects to Emic 2's SOUT pin)
#define txPin   GPIO2  // Serial output (connects to Emic 2's SIN pin)


const char* ssid     = "SSID";
const char* password = "password";

const char* mqtt_server = "mqtt_server";

char message_buff[256];

String TIL[9];

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

// Add TIL message to the String array  
void addTILMessage( byte *message, int msgLength) {
  static int addTILIndex = 0;
  char msg_buff[257];
  
  for (int l=0; l<msgLength; l++)
    msg_buff[l] = message[l];

  msg_buff[msgLength] = '\0' ;

  TIL[addTILIndex] = String(msg_buff);
  TIL[addTILIndex].replace("TIL", "Did you know");
  TIL[addTILIndex].replace("Til", "Did you know");
  
  //Serial.println("Message arrived: ");
  //Serial.println(TIL[addTILIndex]);
  //Serial.println("at location ");
  //Serial.println(addTILIndex);
  
  addTILIndex++;
  if (addTILIndex == 9)
    addTILIndex = 0;
}

void addMotionMessage( byte *message, int msgLength) {
  char msg_buff[257];
  
  for (int l=0; l<msgLength; l++)
    msg_buff[l] = message[l];

  msg_buff[msgLength] = '\0' ;

}

void callback(char* topic, byte* payload, unsigned int length) {
  
  static int TILIndex = 0;
  static int TTSIndex = 0;

  int i = 0;
  
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);

  // Print new MQTT messages
  Serial.println("Message arrived:  topic:" + String(topic) + " Payload: " + msgString);

  // Detect motion event from Kitchen and send TIL string to the Emic 2 TTS card (Serial1)
  if (msgString.indexOf("Kitchen") != -1) {
         Serial1.print("S");
         Serial1.print(TIL[TTSIndex]);
         Serial1.print('\n');
         Serial.println();
         Serial.println("Kitchen motion detected. Sending the following to TTS:");
         Serial.println(TIL[TTSIndex]);
         Serial.println();
         delay(1000);
         TTSIndex++;
         if (TTSIndex == 9)
          TTSIndex = 0;
   }

  // Determine if MQTT Topic is /TIL
  if (strcmp(topic,"/TIL") == 0 ) {
    addTILMessage(payload, length);
  }
  else 
  // Determine if MQTT Topic is /motion
  if (strcmp(topic,"/motion") == 0 ) {
    addMotionMessage(payload, length);
  }
  else
  {
    Serial.println("unknown Topic");
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
      client.subscribe("/TIL");
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
  delay(5000);
  Serial1.print('\n');
  delay(100);
  Serial1.print('V');
  Serial1.print(18);
  Serial1.print('\n');
  delay(100);
  Serial1.print('N');
  Serial1.print(1);
  Serial1.print('\n');
  delay(100);

  // Connect to wifi
  setup_wifi();

  
  // Connect to MQTT Server and subscribe to topic
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  
  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Subsrcribe to the motion & TIL topics on the MQTT Broker
 long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    client.subscribe("/motion");
    client.subscribe("/TIL");
  } 
  
}
