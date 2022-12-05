void log(String str){
  Serial.println("[INFO] " + str);
}
void log_warn(String str){
  Serial.println("[WARN] " + str);
  p_log.clearFields();
  p_log.addField("warn", str);
  //p_log.addField("count", 1);
  if (!client.writePoint(p_log)) {
    Serial.println("[ERROR] " + (String)"InfluxDB write failed (p_log): " + client.getLastErrorMessage());
    //return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  }
}
void log_error(String str){
  Serial.println("[ERROR] " + str);
  p_log.clearFields();
  p_log.addField("error", str);
  //p_log.addField("count", 1);
  if (!client.writePoint(p_log)) {
    Serial.println("[ERROR] " + (String)"InfluxDB write failed (p_log): " + client.getLastErrorMessage());
    //return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  }
}

