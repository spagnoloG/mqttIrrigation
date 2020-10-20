#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// [ENG]
// Code for esp32 devkit v1
// Irrigation controller
// HARD CODED: Only one switch can be turned on simotaneously(več o tem spodaj)
// It connects to any mosquitto broker S Tem dashboardom lahko nadziramo


// [SL]
// Koda za mikroprocesor esp32 devkit v1
// Krmilnik za zalivanje
// HARD CODED: samo eno stikalo je lahko vključeno na enkrat(več o tem spodaj)
// Poveže se preko Wi-Fija na Rpi, na katerem je nameščen
// mosquitto broker in node-red dashboard. S Tem dashboardom lahko nadziramo
// stanja stikal in nastavimo urnike za zalivanje travnika pred hišo (CONA A),
// zalivanje travnika na zahodni strani hiše (CONA B) in namakanje rož okoli
// hiše (CONA C). Protokol, ki se uporablja za komunikacijo med RPijem in
// ESPjem je mqtt, ta protokol je zelo uporaben za IOT projeke.


const char* ssid = "YOUR_WI-FI_SSID";
const char* password = "YOUR_WI-FI_PASSWORD";
// MQTT server IP address
// IP MQTT serverja (RPi)
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
long value = 0;

boolean switch1 = false;
boolean switch2 = false;
boolean switch3 = false;
boolean stop = true;
boolean suMqtt = false;

// WARNING!!!
// USE OTHER PINS!!!!! (IF YOU USE PIN 12, THE ESP WILL CRASH ON BOOT!)
const int relay1 = 12;
const int relay2 = 13;
const int relay3 = 14;

void setup_wifi();
void reconnect();
void callback(char* topic, byte* message, unsigned int length);
void checkZonesOn(char zone);
void checkZonesOff(char zone);
void switchRelays();
void reportCurrentState();

void setup() {
  Serial.begin(9600);
  // Connect to Wi-Fi
  // Poveži Wi-Fi
  setup_wifi();
  // Connect to mqtt broker
  // Poveži se z mqtt serverjem
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
}

void setup_wifi() {

  delay(10);

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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until conected
  // Loopaj dokler se ne povežeš
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Try to connect
    // Poskusi povezati
    if (client.connect("ESP32Client")) {
      suMqtt = true;
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/zoneA");
      client.subscribe("esp32/zoneB");
      client.subscribe("esp32/zoneC");
      client.subscribe("esp32/other");
    } else {
      suMqtt = false;
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before next retry
      // Počakaj 5 sekund pred ponovnim poskusom
      delay(5000);
    }
  }
}

// FUNCTION THAT IS CALLED WHEN MQTT PACKET ARRIVES
// FUNKCIJA, KI SE POKLIČE KO PRIDE MQTT PAKET
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/zoneA") {
    if(messageTemp == "on"){
      // Send zone A to function checkcheckZoneson('A')
      //Pošlji cono A v checkZoneson('A')
      checkZonesOn(topic[10]);
    }
    else if(messageTemp == "off"){
      // Send zone A to function checkcheckZonesoff('A')
      //Pošlji cono A v checkZonesoff('A')
      checkZonesOff(topic[10]);
    }
  }

  if (String(topic) == "esp32/zoneB") {
    if(messageTemp == "on"){
      checkZonesOn(topic[10]);
    }
    else if(messageTemp == "off"){
      checkZonesOff(topic[10]);
    }
  }

  if (String(topic) == "esp32/zoneC") {
    if(messageTemp == "on"){
      checkZonesOn(topic[10]);
    }
    else if(messageTemp == "off"){
      checkZonesOff(topic[10]);
    }
  }

   if (String(topic) == "esp32/other") {
      if(messageTemp == "stop"){
        switch1 = false;
        switch2 = false;
        switch3 = false;
        stop = true;
        switchRelays();
      }else {
        Serial.println("Unknown command!");
      }
   }
  reportCurrentState();
  Serial.println("Repported current state!");
}

// Change if needed! Current setup is only to turn one switch at a time

// Ta funkcija je mogoče nekoliko hard-coded, saj omogoča
// vključitev samo enega stikala. To sem si tako zamislil 
// zaradi varnosti, saj bi se lahko v nasprotnem primeru 
// vključila vsa 3 stikala, kar bi preobremenilo vodni sistem 
void checkZonesOn(char zone) {
  if(zone == 'A') {
    // Če je stikalo že vključeno se vrni, v nasprotnem primeru
    // negiraj ostali 2 stikali in stop spremenljivko. Spremenljivko
    // stikala postavi na vrednost true.
    if(switch1) {
      return;
    } else {
    switch1 = true;
    switch2 = false;
    switch3 = false;
    stop = false;
    }
  }else if (zone == 'B') {
    if(switch2) {
      return;
    } else {
    switch1 = false;
    switch2 = true;
    switch3 = false;
    stop = false;
    }
  } else if(zone == 'C') {
    if(switch3) {
      return;
    } else {
    switch1 = false;
    switch2 = false;
    switch3 = true;
    stop = false;
    }
  }
  switchRelays();
  Serial.print("Zone ");
  Serial.print(zone);
  Serial.println(" activated");
}

// Function to switch off the relay
// Funkcija za izključitev stikala
void checkZonesOff(char zone) {
  if(zone == 'A') {
    if(!switch1) {
      return;
    } else {
    switch1 = false;
    }
  } else if (zone == 'B') {
    if(!switch2) {
      return;
    } else {
    switch2 = false;
    }
  } else if (zone == 'C') {
    if(!switch3) {
      return;
    } else {
    switch3 = false;
    }
  }
  switchRelays();
  Serial.print("Zone ");
  Serial.print(zone);
  Serial.println(" deactivated");
}

void switchRelays() {
  digitalWrite(relay1,switch1?HIGH:LOW);
  digitalWrite(relay2,switch2?HIGH:LOW);
  digitalWrite(relay3,switch3?HIGH:LOW);
}

// Send current-state message to Node-red dashboard(OPTIONAL)
// Sporoči lučkam na dashboardu kako naj svetijo :)
void reportCurrentState() {
  if(switch1) {
    client.publish("esp32/stateA","on");
  } else {
    client.publish("esp32/stateA","off");
  }

  if(switch2) {
    client.publish("esp32/stateB","on");
  } else {
    client.publish("esp32/stateB","off");
  }

  if(switch3) {
    client.publish("esp32/stateC","on");
  } else {
    client.publish("esp32/stateC","off");
  }

  if(stop) {
    client.publish("esp32/stop","on");
  } else {
    client.publish("esp32/stop","off");
  }
}

void loop() {
  // If there is connection lost, try to reconnect
  // V primeru, da izgubimo mqtt povezavo, se poskušaj
  // ponovno povezati
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
  }
}