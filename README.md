# Projet de Désactivation Automatique de Plaque à Induction

## Constat de Départ

Nous avons constaté que notre plaque à induction consomme en mode attente 120W en permanence. Après des recherches, il semble que toutes les plaques à induction consomment une certaine quantité d'énergie en mode attente. Cette consommation est principalement due au mécanisme de sécurité et aux capteurs tactiles qui restent actifs en attente.

## Objectif du Projet

L'objectif de ce projet est de réduire la consommation électrique de notre plaque à induction en mode attente en intégrant un mécanisme de désactivation automatique via notre système de domotique.

## Solution Proposée

## Solutions Proposées

### Solution avec ESP32

Nous proposons d'utiliser un ESP32 pour reconfigurer un interrupteur en cuisine, câblé en basse tension, afin d'envoyer un signal à un circuit domotique qui active un relais sur la phase 32 ampères de la plaque à induction.

#### Matériel Nécessaire

- ESP32
- Interrupteur
- Relais SSR-40
- Capteur SCT013
- Breadboard et câbles de connexion

#### Connexions

1. **ESP32** :
   - Pin GND à la ligne de masse de la breadboard.
   - Pin 3.3V à la ligne d'alimentation de la breadboard.

2. **Interrupteur** :
   - Une borne à un pin GPIO de l'ESP32 (ex. GPIO 12).
   - Une borne à la ligne de masse.

3. **Relais SSR-40** :
   - VCC du relais au 3.3V de l'ESP32.
   - GND du relais au GND de l'ESP32.
   - IN du relais à un pin GPIO de l'ESP32 (ex. GPIO 13).

4. **Capteur SCT013** :
   - Connecter les fils du capteur aux extrémités d'une résistance de charge.
   - Un côté de la résistance à une entrée analogique de l'ESP32 (ex. ADC1_CHANNEL_0).
   - L'autre côté de la résistance à la ligne de masse.

#### Code Exemple pour ESP32

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

// Configuration WiFi
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Configuration MQTT
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";
const char* mqtt_user = "YOUR_MQTT_USERNAME";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";
const char* switch_topic = "home/kitchen/induction_switch";
const char* power_topic = "home/kitchen/induction_power";

WiFiClient espClient;
PubSubClient client(espClient);

const int switchPin = 12;
const int relayPin = 13;
const int sensorPin = 34; // ADC1_CHANNEL_0

void setup() {
  pinMode(switchPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int switchState = digitalRead(switchPin);
  int sensorValue = analogRead(sensorPin);

  if (switchState == HIGH) {
    digitalWrite(relayPin, HIGH);
    client.publish(switch_topic, "ON");
  } else {
    digitalWrite(relayPin, LOW);
    client.publish(switch_topic, "OFF");
  }

  char msg[50];
  sprintf(msg, "%d", sensorValue);
  client.publish(power_topic, msg);

  delay(1000);
}
```

### Solution avec Raspberry Pi 4

Une autre solution consiste à utiliser un Raspberry Pi 4 pour gérer l'interrupteur, le relais SSR-40 et le capteur SCT013.

#### Matériel Nécessaire

- Raspberry Pi 4
- Relais SSR-40
- Interrupteur
- Capteur SCT013
- Breadboard et câbles de connexion

#### Installation de Home Assistant en Docker

1. **Préparer le Raspberry Pi :**
   - Installe Raspbian OS.
   - Installe Docker :

     ```bash
     curl -fsSL https://get.docker.com -o get-docker.sh
     sh get-docker.sh
     sudo usermod -aG docker pi
     ```

   - Redémarre le Raspberry Pi.

2. **Installer Home Assistant :**
   - Crée un répertoire pour Home Assistant :

     ```bash
     mkdir /home/pi/homeassistant
     ```

   - Lance le conteneur Docker pour Home Assistant :

     ```bash
     docker run -d --name homeassistant --privileged --restart=unless-stopped -e TZ=YOUR_TIME_ZONE -v /home/pi/homeassistant:/config --network=host ghcr.io/home-assistant/home-assistant:stable
     ```

#### Connexions

1. **Relais SSR-40** :
   - VCC du relais au 3.3V du Raspberry Pi (Pin 1).
   - GND du relais au GND du Raspberry Pi (Pin 6).
   - IN du relais au GPIO 17 (Pin 11).

2. **Interrupteur** :
   - Une borne à GPIO 27 (Pin 13).
   - Une borne à GND (Pin 9).

3. **Capteur SCT013** :
   - Connecte les fils du capteur aux extrémités d'une résistance de charge et à un ADC (comme MCP3008) relié au Raspberry Pi.

#### Exemple de Code Python

```python
import RPi.GPIO as GPIO
import spidev
import time
import paho.mqtt.client as mqtt

# Configuration MQTT
mqtt_broker = "YOUR_MQTT_BROKER_IP"
mqtt_user = "YOUR_MQTT_USERNAME"
mqtt_password = "YOUR_MQTT_PASSWORD"
switch_topic = "home/kitchen/induction_switch"
power_topic = "home/kitchen/induction_power"

# Configuration GPIO
SWITCH_PIN = 27
RELAY_PIN = 17
ADC_CHANNEL = 0

GPIO.setmode(GPIO.BCM)
GPIO.setup(SWITCH_PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(RELAY_PIN, GPIO.OUT)

spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 1350000

def read_adc(channel):
    adc = spi.xfer2([1, (8 + channel) << 4, 0])
    data = ((adc[1] & 3) << 8) + adc[2]
    return data

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

client = mqtt.Client()
client.username_pw_set(mqtt_user, mqtt_password)
client.on_connect = on_connect
client.connect(mqtt_broker, 1883, 60)
client.loop_start()

try:
    while True:
        switch_state = GPIO.input(SWITCH_PIN)
        sensor_value = read_adc(ADC_CHANNEL)
        
        if switch_state == GPIO.HIGH:
            GPIO.output(RELAY_PIN, GPIO.HIGH)
            client.publish(switch_topic, "ON")
        else:
            GPIO.output(RELAY_PIN, GPIO.LOW)
            client.publish(switch_topic, "OFF")
            
        client.publish(power_topic, str(sensor_value))
        
        time.sleep(1)
except KeyboardInterrupt:
    pass
finally:
    GPIO.cleanup()
    spi.close()
    client.loop_stop()
    client.disconnect()

```

### Solution avec Arduino MKR 1010 WIFI

Enfin, une solution basée sur l'Arduino MKR 1010 WIFI peut être mise en place pour contrôler l'interrupteur, le relais SSR-40, et le capteur SCT013.

#### Matériel Nécessaire

- Arduino MKR 1010 WIFI
- Relais SSR-40
- Interrupteur
- Capteur SCT013
- Breadboard et câbles de connexion

#### Connexions

1. **Relais SSR-40** :
   - VCC du relais au 5V de l'Arduino.
   - GND du relais au GND de l'Arduino.
   - IN du relais à un pin digital (ex. D2).

2. **Interrupteur** :
   - Une borne à un pin digital (ex. D3).
   - Une borne à GND.

3. **Capteur SCT013** :
   - Connecter les fils du capteur aux extrémités d'une résistance de charge et à une entrée analogique de l'Arduino (ex. A0).

#### Exemple de Code pour Arduino MKR 1010 WIFI

```cpp
#include <WiFiNINA.h>
#include <PubSubClient.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* mqtt_server = "YOUR_MQTT_BROKER_IP";
const char* mqtt_user = "YOUR_MQTT_USERNAME";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";
const char* switch_topic = "home/kitchen/induction_switch";
const char* power_topic = "home/kitchen/induction_power";

WiFiClient espClient;
PubSubClient client(espClient);

const int switchPin = 3;
const int relayPin = 2;
const int sensorPin = A0;

void setup() {
  pinMode(switchPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int switchState = digitalRead(switchPin);
  int sensorValue = analogRead(sensorPin);

  if (switchState == HIGH) {
    digitalWrite(relayPin, HIGH);
    client.publish(switch_topic, "ON");
  } else {
    digitalWrite(relayPin, LOW);
    client.publish(switch_topic, "OFF");
  }

  char msg[50];
  sprintf(msg, "%d", sensorValue);
  client.publish(power_topic, msg);

  delay(1000);
}

```
# SCHEMA
![Location of Fritzing Schema](doc/schema.png)
