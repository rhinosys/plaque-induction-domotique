#include "arduino_secrets.h"
#include <EmonLib.h>
#include "thingProperties.h"

#include <Arduino_MKRIoTCarrier.h>
MKRIoTCarrier carrier;


// DOC https://makeabilitylab.github.io/physcomp/arduino/buttons.html
// https://www.lg-system.fr/publication.php?viewPublication=23
// https://www.youtube.com/watch?v=PYOV9O41ROQ
// https://www.moussasoft.com/sct-013-000-capteur-de-courant-ac-avec-arduino/

EnergyMonitor emon1;

#define TENSION 226
#define PIN_SCT013 1
#define RELAY_PIN 5   // Choisissez la broche pour le relais
#define BUTTON_PIN 2  // Choisissez la broche pour le bouton

float energyAccumulated = 0;

void setup()  
{  

  
  pinMode(RELAY_PIN, OUTPUT);   // Définir la broche du relais comme sortie
  pinMode(BUTTON_PIN, INPUT);   // Définir la broche du bouton comme entrée
  digitalWrite(RELAY_PIN, LOW); // Assurez-vous que le relais est désactivé au démarrage
  
  Serial.begin(9600);
  while (!Serial);
  delay(1500);
  
  Serial.println("\n\nTest du détecteur de courant");
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  while (!ArduinoCloud.connected()) {
    ArduinoCloud.update();
    delay(500);
  }

  CARRIER_CASE = true;
  carrier.begin();
  carrier.display.setRotation(0);

  emon1.current(PIN_SCT013, 21);
}

void loop()                     
{
  ArduinoCloud.update();

  double Irms = emon1.calcIrms(1480);
  double power = Irms * TENSION;
  energyAccumulated += power / 3600;

  currentDisplay = Irms;
  powerDisplay = power;
  energyDisplay = energyAccumulated;

  Serial.print("Courant : ");
  Serial.print(Irms);
  Serial.println(" A");
  Serial.print("Puissance : ");
  Serial.print(power);
  Serial.println(" W");
  Serial.print("Énergie cumulée : ");
  Serial.print(energyAccumulated);
  Serial.println(" kWh");

  Serial.print("button state");
  Serial.println(digitalRead(BUTTON_PIN));
  if (digitalRead(BUTTON_PIN) == HIGH) { // Si le bouton est pressé
    Serial.println(" BUTTON_PIN HIGH");
    digitalWrite(RELAY_PIN, HIGH);       // Activer le relais
    delay(10000);                         // Délai pour stabilité (ajustable)
    digitalWrite(RELAY_PIN, LOW);        // Désactiver le relais
  }
  else
  {
     Serial.println(" BUTTON_PIN LOW");
  }

  carrier.display.fillScreen(ST77XX_WHITE);
  carrier.display.setTextColor(ST77XX_RED);
  carrier.display.setTextSize(2);

  carrier.display.setCursor(30, 110);
  carrier.display.print("Courant : ");
  carrier.display.print(Irms);
  carrier.display.print(" A");



  delay(10);  // Attente avant la prochaine mesure
}
