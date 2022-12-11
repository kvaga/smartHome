#include "debug.h"
String get_debug_info_as_string(){
  #if defined(DEBUG_MODE)
    return (String) " [DEBUG] ramfree="+ramfree() + ", flashfree=" + flashfree();
     //Serial.println(ramfree(), DEC);
//      Serial.println(flashfree(), DEC);
  #else
    return "";
  #endif
}
void log(String str){
  Serial.println("[INFO] " + str + get_debug_info_as_string());
}
void log_warn(String str){
  Serial.println("[WARN] " + str + get_debug_info_as_string());
  // p_log.clearFields();
  // p_log.addField("warn", str);
  //p_log.addField("count", 1);
  // if (!influxdbWritePoint(p_log)) {
  //   Serial.println("[ERROR] " + (String)"InfluxDB write failed (p_log): " + client.getLastErrorMessage());
  //   //return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  // }
}
void log_error(String str){
  Serial.println("[ERROR] " + str + get_debug_info_as_string());
  // p_log.clearFields();
  // p_log.addField("error", str);
  // //p_log.addField("count", 1);
  // if (!influxdbWritePoint(p_log)) {
  //   Serial.println("[ERROR] " + (String)"InfluxDB write failed (p_log): " + client.getLastErrorMessage());
  //   //return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  // }
}

void log_debug(String str){
    #if defined(DEBUG_MODE)
      Serial.println("[ERROR] " + str + get_debug_info_as_string());
    #endif
}


