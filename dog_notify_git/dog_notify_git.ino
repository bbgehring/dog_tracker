/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h> 

// Update these with values suitable for your network.
const char* ssid = "insert wifi name here";
const char* password = "insert wifi password here";
const char* mqtt_server = "insert your home assistant IP address here";

WiFiClient espClient;
PubSubClient client(espClient);

#define led1_gate 5 //D1
#define led2_gate 4 //D1
#define led3_gate 0 //D1

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

// Flashes the D1 light a couple times to show wifi has connected
  digitalWrite(led1_gate, HIGH);
  delay(200);
  digitalWrite(led1_gate, LOW);
  delay(200);
  digitalWrite(led1_gate, HIGH);
  delay(200);
  digitalWrite(led1_gate, LOW);
  delay(50);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  payload[length] = '\0';
  String message = (char*)payload;
  String strtopic = (char*)topic;
  Serial.println(message);

  // This is the meat and potatoes - this listens for the topics below and 
  // turns the LED either off or on depending on if it recieves a 1 or 0
  
  if ((strtopic == "dog_listen/fed") && message == "1") {
    digitalWrite(led1_gate, LOW);   // Turn the LED on
  } 
    else if ((strtopic == "dog_listen/fed") && message == "0") {
    digitalWrite(led1_gate, HIGH);  // Turn the LED off 
  } 
    else if ((strtopic == "dog_listen/med") && message == "1") {
    digitalWrite(led2_gate, LOW);   // Turn the LED on 
  }
    else if ((strtopic == "dog_listen/med") && message == "0") {
    digitalWrite(led2_gate, HIGH);  // Turn the LED off
  } 
    else if ((strtopic == "dog_listen/walk") && message == "1") {
    digitalWrite(led3_gate, LOW);   // Turn the LED on 
  } 
    else if ((strtopic == "dog_listen/walk") && message == "0") {
    digitalWrite(led3_gate, HIGH);  // Turn the LED off 
  }
  }
  
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "dog_notify";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...do
      client.publish("dog_notify", "Ready");
      // ... and resubscribe
      // these are the listening topics
      client.subscribe("dog_listen/fed");
      client.subscribe("dog_listen/med");
      client.subscribe("dog_listen/walk");
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
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(led1_gate, OUTPUT);
  pinMode(led2_gate, OUTPUT);
  pinMode(led3_gate, OUTPUT);
  digitalWrite(led1_gate, LOW);
  digitalWrite(led2_gate, LOW);
  digitalWrite(led3_gate, LOW);

// you will need to setup a username and password in your mosquitto setup in HA 
// then add the username and password for that below so your ESP module can communicate with the broker
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.connect("dog_notify", "this is your mosquitto broker username", "mosquitto broker password");
  client.publish("dog/notify", "prime");
  // these are the topics of each of your mqtt switches in HA
  client.subscribe("dog_listen/fed");
  client.subscribe("dog_listen/med");
  client.subscribe("dog_listen/walk");
  client.setCallback(callback);

  // OTA code
  ArduinoOTA.setHostname("dog"); //Replace HOSTNAMEHERE with the name you want to give your module
  ArduinoOTA.setPassword((const char *)"1234");//REPLACE PASSWORDHERE with whatever password you want for your OTA updates
  ArduinoOTA.begin();
  ArduinoOTA.onStart([]() {
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    digitalWrite(led1_gate, HIGH); // flash an LED just to know OTA is progressing.
    delay(200);
    digitalWrite(led1_gate, LOW);
    delay(200); 
  });
  ArduinoOTA.onEnd([]() {
  });
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
