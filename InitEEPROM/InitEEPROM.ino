#include <EEPROM.h>
#include "../bath_fan_humidity_control/eeprom.h"
#include <SoftwareSerial.h>;

const int EEPROM_HOSTNAME=2;

void setup(){
  Serial.begin(115200);
  String hostname = "kitchen-humidity";
  // String hostname = "bath-humidity";
  // String hostname = "test-humidity";

  save(EEPROM_HOSTNAME, hostname);
}

void loop() {
  Serial.println("[INFO] hostname: " + read(EEPROM_HOSTNAME));
  delay(5000);
}