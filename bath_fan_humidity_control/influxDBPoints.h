// InfluxDB data point
// Point sensor("wifi_status");
Point p_log("log");
Point p_sensor_dth11("dth11");
// Point p_error_code("error_code");
Point p_monitoring("monitoring");
Point p_current_humidity_threshold("current_humidity_threshold");

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


String ipAddress2String(const IPAddress& ipAddress)
{
    return  String(ipAddress[0]) + String(".") + \
            String(ipAddress[1]) + String(".") + \
            String(ipAddress[2]) + String(".") + \
            String(ipAddress[3])  ;
}

String escapeValueSymbolsOfInfluxDBPoint(String value){
  value.replace(" ", "\\ ");
}
void addFieldToPoint(Point p, String key, String value){
  p.addField(key, escapeValueSymbolsOfInfluxDBPoint(value));
}

void initInfluxDBPoints(){
  // Setup InfluXDB tags
  p_sensor_dth11.addTag("device", DEVICE);
  p_sensor_dth11.addTag("SSID", WiFi.SSID());
  p_sensor_dth11.addTag("ip", ipAddress2String(WiFi.localIP()));
  p_sensor_dth11.addTag("host",WiFi.hostname().c_str());
  p_sensor_dth11.addTag("localtion","home2");

  // p_error_code.addTag("device", DEVICE);
  // p_error_code.addTag("SSID", WiFi.SSID());
  // p_error_code.addTag("ip", ipAddress2String(WiFi.localIP()));
  // p_error_code.addTag("host",WiFi.hostname().c_str());
  // p_error_code.addTag("location","home2");

  p_monitoring.addTag("device", DEVICE);
  p_monitoring.addTag("SSID", WiFi.SSID());
  p_monitoring.addTag("ip", ipAddress2String(WiFi.localIP()));
  p_monitoring.addTag("host",WiFi.hostname().c_str());
  p_monitoring.addTag("location","home2");
  p_monitoring.addTag("ESPChipId", (String)ESP.getChipId());// returns the ESP8266 chip ID as a 32-bit integer.
  p_monitoring.addTag("ESPCoreVersion", ESP.getCoreVersion());//  returns a String containing the core version.
  p_monitoring.addTag("ESPSdkVersion", ESP.getSdkVersion());//  returns the SDK version as a char.
  p_monitoring.addTag("ESPCpuFreqMHz", (String)ESP.getCpuFreqMHz());//  returns the CPU frequency in MHz as an unsigned 8-bit integer.
  p_monitoring.addTag("ESPSketchSize", (String)ESP.getSketchSize());//  returns the size of the current sketch as an unsigned 32-bit integer.
  p_monitoring.addTag("ESPFreeSketchSpace", (String)ESP.getFreeSketchSpace());//  returns the free sketch space as an unsigned 32-bit integer.
  p_monitoring.addTag("ESPSketchMD5", ESP.getSketchMD5());//  returns a lowercase String containing the MD5 of the current sketch.
  p_monitoring.addTag("ESPFlashChipId", (String)ESP.getFlashChipId());//  returns the flash chip ID as a 32-bit integer.
  p_monitoring.addTag("ESPFlashChipSizeBytes", (String)ESP.getFlashChipSize());//  returns the flash chip size, in bytes, as seen by the SDK (may be less than actual size).
  p_monitoring.addTag("ESPFlashChipRealSizeBytes", (String)ESP.getFlashChipRealSize());//  returns the real chip size, in bytes, based on the flash chip ID.
  p_monitoring.addTag("ESPFlashChipSpeedHz", (String)ESP.getFlashChipSpeed(/*void*/));//  returns the flash chip frequency, in Hz.

  p_current_humidity_threshold.addTag("device", DEVICE);
  p_current_humidity_threshold.addTag("SSID", WiFi.SSID());
  p_current_humidity_threshold.addTag("ip", ipAddress2String(WiFi.localIP()));
  p_current_humidity_threshold.addTag("host",WiFi.hostname().c_str());
  p_current_humidity_threshold.addTag("location","home2");

  p_log.addTag("device", DEVICE);
  p_log.addTag("SSID", WiFi.SSID());
  p_log.addTag("ip", ipAddress2String(WiFi.localIP()));
  p_log.addTag("host",WiFi.hostname().c_str());
  p_log.addTag("location","home2");
}

