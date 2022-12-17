// InfluxDB data point
// Point sensor("wifi_status");
// Point p_log("log");
// Point p_error_code("error_code");
// Point p_monitoring("monitoring");

// int send_error_code(int errorCode){
//     p_error_code.clearFields();
//     p_error_code.addField("_code" , errorCode);
//   log((String)"Writing error code: " + p_error_code.toLineProtocol());
  
//   if (!client.writePoint(p_error_code)) {
//     log_error((String)"InfluxDB write failed (p_error_code): " + client.getLastErrorMessage());
//     return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
//   }
//   return PR_SUCCESS;
// }


// String ipAddress2String(const IPAddress& ipAddress)
// {
//     return  String(ipAddress[0]) + String(".") + \
//             String(ipAddress[1]) + String(".") + \
//             String(ipAddress[2]) + String(".") + \
//             String(ipAddress[3])  ;
// }

String escapeValueSymbolsOfInfluxDBPoint(String value){
  value.replace(F(" "), F("\\ "));
}
void addFieldToPoint(Point p, String key, String value){
  p.addField(key, escapeValueSymbolsOfInfluxDBPoint(value));
}
const char* PROGMEM POINT_NAME_DTH11="dth11";
Point p_sensor_dth11(POINT_NAME_DTH11);
// Point p_current_humidity_threshold("current_humidity_threshold");

/*
Point initInfluxDBPoint(char* pointName){
  Point p(pointName);
  p.addTag("device", DEVICE);
  // p.addTag("SSID", WiFi.SSID());
  // p.addTag("ip", ipAddress2String(WiFi.localIP()));
  p.addTag("host",WiFi.hostname().c_str());
  p.addTag("location", LOCATION);
  return p;
}
*/




void initInfluxDBPoints(){
  // Setup InfluXDB tags
  // p_sensor_dth11.addTag("device", DEVICE);
  // p_sensor_dth11.addTag("SSID", WiFi.SSID());
  // p_sensor_dth11.addTag("ip", ipAddress2String(WiFi.localIP()));
  p_sensor_dth11.addTag(F("host"),WiFi.hostname().c_str());
  p_sensor_dth11.addTag(F("localtion"),"home2");

  // p_error_code.addTag("device", DEVICE);
  // p_error_code.addTag("SSID", WiFi.SSID());
  // p_error_code.addTag("ip", ipAddress2String(WiFi.localIP()));
  // p_error_code.addTag("host",WiFi.hostname().c_str());
  // p_error_code.addTag("location","home2");

  // p_monitoring.addTag("device", DEVICE);
  // p_monitoring.addTag("SSID", WiFi.SSID());
  // p_monitoring.addTag("ip", ipAddress2String(WiFi.localIP()));
  // p_monitoring.addTag("host",WiFi.hostname().c_str());
  // p_monitoring.addTag("location","home2");
  // p_monitoring.addTag("ESPChipId", (String)ESP.getChipId());// returns the ESP8266 chip ID as a 32-bit integer.
  // p_monitoring.addTag("ESPCoreVersion", ESP.getCoreVersion());//  returns a String containing the core version.
  // p_monitoring.addTag("ESPSdkVersion", ESP.getSdkVersion());//  returns the SDK version as a char.
  // p_monitoring.addTag("ESPCpuFreqMHz", (String)ESP.getCpuFreqMHz());//  returns the CPU frequency in MHz as an unsigned 8-bit integer.
  // p_monitoring.addTag("ESPSketchSize", (String)ESP.getSketchSize());//  returns the size of the current sketch as an unsigned 32-bit integer.
  // p_monitoring.addTag("ESPFreeSketchSpace", (String)ESP.getFreeSketchSpace());//  returns the free sketch space as an unsigned 32-bit integer.
  // p_monitoring.addTag("ESPSketchMD5", ESP.getSketchMD5());//  returns a lowercase String containing the MD5 of the current sketch.
  // p_monitoring.addTag("ESPFlashChipId", (String)ESP.getFlashChipId());//  returns the flash chip ID as a 32-bit integer.
  // p_monitoring.addTag("ESPFlashChipSizeBytes", (String)ESP.getFlashChipSize());//  returns the flash chip size, in bytes, as seen by the SDK (may be less than actual size).
  // p_monitoring.addTag("ESPFlashChipRealSizeBytes", (String)ESP.getFlashChipRealSize());//  returns the real chip size, in bytes, based on the flash chip ID.
  // p_monitoring.addTag("ESPFlashChipSpeedHz", (String)ESP.getFlashChipSpeed());//  returns the flash chip frequency, in Hz.

  // p_current_humidity_threshold.addTag("device", DEVICE);
  // p_current_humidity_threshold.addTag("SSID", WiFi.SSID());
  // p_current_humidity_threshold.addTag("ip", ipAddress2String(WiFi.localIP()));
  // p_current_humidity_threshold.addTag("host",WiFi.hostname().c_str());
  // p_current_humidity_threshold.addTag("location","home2");

  // p_log.addTag("device", DEVICE);
  // p_log.addTag("SSID", WiFi.SSID());
  // p_log.addTag("ip", ipAddress2String(WiFi.localIP()));
  // p_log.addTag("host",WiFi.hostname().c_str());
  // p_log.addTag("location","home2");
}


/*
int influxdbWritePoint(Point p){
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
    return -2;
  }
 if (!client.isBufferEmpty()) {
     // Write all remaining points to db
     client.flushBuffer();
 }
 int status = client.writePoint(p); // Check whether buffer in not empty
 client.flushBuffer();
 return status;
}
*/


/*
long sendMonitoringDataTimer=0;
int sendMonitoringData(){
  if(millis()-sendMonitoringDataTimer>5000){
    sendMonitoringDataTimer=millis();
  //F function does the same and is now a built in library, in IDE > 1.0.0
   Serial.println(F("Free RAM = "));
  long  fh = ESP.getFreeHeap();
  char  fhc[20];
  ltoa(fh, fhc, 10);
  Serial.println(fh);
  p_monitoring.clearFields();
  p_monitoring.addField("FreeHeapBytes", fh);
  //fields:
  //ESP.getFreeHeap() returns a String containing the last reset reason in human readable format.
  p_monitoring.addField("HeapFragmentationPercent", ESP.getHeapFragmentation()); //returns the fragmentation metric (0% is clean, more than ~50% is not harmless)
  p_monitoring.addField("MaxFreeBlockSize", ESP.getMaxFreeBlockSize()); // returns the largest contiguous free RAM block in the heap, useful for checking heap fragmentation. NOTE: Maximum malloc() -able block will be smaller due to memory manager overheads.
  //p_monitoring.addField("FlashCRC", ESP.checkFlashCRC()); // Alert!!! calculates the CRC of the program memory (not including any filesystems) and compares it to the one embedded in the image. If this call returns false then the flash has been corrupted. At that point, you may want to consider trying to send a MQTT message, to start a re-download of the application, blink a LED in an SOS pattern, etc. However, since the flash is known corrupted at this point there is no guarantee the app will be able to perform any of these operations, so in safety critical deployments an immediate shutdown to a fail-safe mode may be indicated.
  //p_monitoring.addField("Vcc", ESP.getVcc());
// ESP.getVcc() may be used to measure supply voltage. ESP needs to reconfigure the ADC at startup in order for this feature to be available. Add the following line to the top of your sketch to use getVcc:
// ADC_MODE(ADC_VCC);
// TOUT pin has to be disconnected in this mode.
// Note that by default ADC is configured to read from TOUT pin using analogRead(A0), and ESP.getVCC() is not available.

  log((String)F("Writing p_monitoring: ") + p_monitoring.toLineProtocol());

  // If no Wifi signal, try to reconnect it
//  if (wifi_connection_lost()){
//    return -1;
//  }
  // Write point
  //if (!client.writePoint(p_monitoring)) {
    if(!influxdbWritePoint(p_monitoring)){
    log_error((String)F("InfluxDB write failed (p_monitoring: [")+p_monitoring.toLineProtocol()+F("], httpCode: ")+client.getLastStatusCode()+F("): ") + client.getLastErrorMessage());
    return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  }
   // print how much RAM is available.
//   Serial.println(ramfree(), DEC);
//      Serial.println(flashfree(), DEC);
   // print how much flash is available.
  }
}
*/

/*
int send_current_humidity_threshold(int currentHumidityThreshold){
  //Point p_current_humidity_threshold("current_humidity_threshold");
  Point p_current_humidity_threshold = initInfluxDBPoint("current_humidity_threshold");
  p_current_humidity_threshold.clearFields();
  p_current_humidity_threshold.addField("value" , currentHumidityThreshold);
  log((String)"Send current humidity threshold: " + p_current_humidity_threshold.toLineProtocol());
  if (!influxdbWritePoint(p_current_humidity_threshold)) {
    log_error((String)"InfluxDB write failed (currentHumidityThreshold): " + client.getLastErrorMessage());
    return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  }
  return PR_SUCCESS;
}
*/