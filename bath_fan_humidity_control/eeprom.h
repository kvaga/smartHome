/*
   EEPROM Write

   Stores values read from analog input 0 into the EEPROM.
   These values will stay in the EEPROM when the board is
   turned off and may be retrieved later by another sketch.
*/

#include <EEPROM.h>

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
int addr = 0;
byte value;
//int EEPROM_HUMIDIDITY=0;

void save(int address, int humidity) {
  // need to divide by 4 because analog inputs range from
  // 0 to 1023 and each byte of the EEPROM can only hold a
  // value from 0 to 255.
  //int val = analogRead(A0) / 4;

  // write the value to the appropriate byte of the EEPROM.
  // these values will remain there when the board is
  // turned off.
  if( EEPROM.read(address) != humidity ){
      EEPROM.write(address, humidity);
  }
  // advance to the next address.  there are 512 bytes in
  // the EEPROM, so go back to 0 when we hit 512.
  // save all changes to the flash.
  //addr = addr + 1;
  if (addr == EEPROM.length()) {
    addr = 0;
    
    delay(100);
  }
  if (EEPROM.commit()) {
      Serial.println("EEPROM successfully committed");
    } else {
      Serial.println("ERROR! EEPROM commit failed");
  }
  log((String)"EEPROM put [" + String(address) + "]: " + humidity);
}

 

byte read(int _addr) {
  // read a byte from the current address of the EEPROM
  // value = EEPROM.read(addr);

  // Serial.print(addr);
  // Serial.print("\t");
  // Serial.print(value, DEC);
  // Serial.println();

  // advance to the next address of the EEPROM
  //addr = addr + 1;

  // there are only 512 bytes of EEPROM, from 0 to 511, so if we're
  // on address 512, wrap around to address 0
  //if (addr == 512) {
  //  addr = 0;
  //}
  
  //delay(500);
  //log((String)"EEPROM read [" + String(_addr) + "]: " + EEPROM.read(_addr));
  return EEPROM.read(_addr);
}


void clear(){
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }
  log("EEPROM cleared");
}
