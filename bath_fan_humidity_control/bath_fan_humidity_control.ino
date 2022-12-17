/**
 * ************************************************************
 * ******    SMART HOME: Check humidity and manage relay ******
 * ************************************************************
 * 
 * ########
 * # WiFi #
 * ########
 * Enter WiFi and InfluxDB parameters below
 * // WiFi AP SSID
 * #define WIFI_SSID "defined in credentials.h"
 * // WiFi password
 * #define WIFI_PASSWORD "defined in credentials.h"
 *
 * ############
 * # InfluxDB #
 * ############
 * Demonstrates connection to any InfluxDB instance accesible via:
 *  - unsecured http://...
 *  - secure https://... (appropriate certificate is required)
 *  - InfluxDB 2 Cloud at https://cloud2.influxdata.com/ (certificate is preconfigured, 
 *  but it's recomended to download one more time certificate for the INFLUXDB_URL and save to the InfluxDbClient.h)
 * Measures signal level of the actually connected WiFi network
 * This example demonstrates time handling, secure connection and measurement writing into InfluxDB
 * Data can be immediately seen in a InfluxDB 2 Cloud UI - measurement wifi_status
 * Src: https://github.com/bonitoo-io/influxdb-client-arduino/blob/master/examples/SecureWrite/SecureWrite.ino
 * 
 * // InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
 * #define INFLUXDB_URL "https://europe-west1-1.gcp.cloud2.influxdata.com"
 * // InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
 * #define INFLUXDB_TOKEN "defined in credentials.h"
 * // InfluxDB v2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
 * #define INFLUXDB_ORG "defined in credentials.h"
 * // InfluxDB v2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
 * #define INFLUXDB_BUCKET "defined in credentials.h"
 * 
 **/

/**
Fix of OOM: https://github.com/esp8266/Arduino/issues/6811
*/
// #define _stackSize (6748/4) 
//#define INFLUXDB_CLIENT_DEBUG_ENABLE

#if defined(ESP32)
    #include <WiFiMulti.h>
    WiFiMulti wifiMulti;
    #define DEVICE "ESP32"
#elif defined(ESP8266)
//    #include <ESP8266WiFiMulti.h>
//    ESP8266WiFiMulti wifiMulti;
    #include <ESP8266WiFi.h>
    #include <WiFiClient.h>
    //#include <ESP8266WebServer.h>
    #include <ESP8266mDNS.h>
    #define DEVICE "ESP8266"

    
    /**
     *     __________________
     *    |                  |
     *    |      RELAY       | 
     *    |                  |
     *    |__________________|
     *      VCC| GND|  IN|
     *     
     *         |    |    |
     *         |    |    |
     *         |    |    |
     *       3V3 GND D3(0)
     *      ___|____|____|___
     *     |                 |
     *     |     ESP8266     |
     *     |_________________|
     */
    const int RELAY_PIN = 0;  //D3
                         
#endif
#include "constants.h"
#include <credentials.h>

#include "InfluxDbClient.h"
#include "InfluxDbCloud.h"
#include "lib_log.h"
#include "eeprom.h"
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
#include "influxDBPoints.h"

#include "webserver.h"

void initWifi(){
  log(F("Init of wifi started"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.hostname(HOSTNAME /*HOSTNAME.c_str()*/);
  //wifiConnect();
  //wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  log((String)F("Connected to wifi: ") + WiFi.SSID() +  F(", hostname=") + WiFi.hostname().c_str());
  log((String)F("ip=") + WiFi.localIP().toString());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  log(F("Init of wifi stoped"));
}

// InfluxDB client instance with preconfigured InfluxCloud certificate


#include <SoftwareSerial.h>;
//#include <MemoryFree.h>;


unsigned long timing;
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"



#include <SimpleDHT.h>
SimpleDHT11 dht11;

byte currentHumidityThreshold=0;//=55;//HUMIDITY_THRESHOLD_DEFAULT_VALUE;

/**
 *     ______________
 *    |              |
 *    |     DTH11    | 
 *    |              |
 *    |______________|
 *      D5| 3V3| GND|
 *     
 *        |  |  |
 *        |  |  |
 *        |  |  |
 *      GND 3V3 D5(14)
 *       _|__|__|_
 *      |         |
 *      | ESP8266 |
 *      |_________|
 */
const int PROGMEM pinDHT11 = 14; // пин data сенсора dth14 к D5 esp8266
// void log(String str){
//   Serial.println("[INFO] " + str);
// }
// void log_warn(String str){
//   Serial.println("[WARN] " + str);
// }
// void log_error(String str){
//   Serial.println("[ERROR] " + str);
// }

// const PROGMEM __FlashStringHelper* TIMESYNC_CONFIGTIME_SITE1 = "pool.ntp.org";
// const PROGMEM __FlashStringHelper* TIMESYNC_CONFIGTIME_SITE2 = "time.nis.gov";

void timeSync() {
  // Synchronize UTC time with NTP servers
  // Accurate time is necessary for certificate validaton and writing in batches
  configTime(0, 0, "pool.ntp.org", "time.nis.gov");
  // Set timezone
  setenv("TZ", TZ_INFO, 1);

  // Wait till time is synced
  log(F("Syncing time"));
  int i = 0;
  while (time(nullptr) < 1000000000ul && i < 100) {
    Serial.print(F("."));
    delay(100);
    i++;
  }
  log(F(""));
  // Show time
  time_t tnow = time(nullptr);
  log((String)F("Synchronized time: ") + String(ctime(&tnow)));
}

void initInfluxDB(){
  log(F("Init of influXDB started"));
  initInfluxDBPoints();
  // Sync time for certificate validation
  timeSync();

  // Check server connection
  if (client.validateConnection()) {
    log((String)F("Connected to InfluxDB: ") + client.getServerUrl());
    //Get Current Hostname
  } else {
    log_error((String)F("InfluxDB connection failed: ") + client.getLastErrorMessage());
  }
  log(F("Init of influxDB finished"));
}

long wifiConnectTimer=0;
void wifiConnect(){
  if(millis()-wifiConnectTimer>5000){
    wifiConnectTimer=millis();
  
//  if(wifiMulti.run() != WL_CONNECTED){
    uint8_t status = WiFi.waitForConnectResult();
    switch (status){
      case WL_NO_SSID_AVAIL:
        log(F("Connection status text: configured SSID cannot be reached")); break;
      case WL_CONNECTED:
        log(F("Connection status text: connection successfully established"));
        return;
        break;
      case WL_CONNECT_FAILED:
        Serial.print(F("Connection status text: connection failed"));
        break;
      case WL_NO_SHIELD:
        Serial.print(F("Connection status text: WL_NO_SHIELD")); break;  
      case WL_IDLE_STATUS:
        Serial.print(F("Connection status text: WL_IDLE_STATUS")); break;
      case WL_SCAN_COMPLETED:
        Serial.print(F("Connection status text: WL_SCAN_COMPLETED")); break;
      case WL_DISCONNECTED:
        Serial.print(F("Connection status text: WL_DISCONNECTED")); break;
      case WL_CONNECTION_LOST:
        Serial.print(F("Connection status text: WL_CONNECTION_LOST")); break;
    }
    Serial.println();
    Serial.print(F("Connection status digital code: %d\n")); Serial.println(WiFi.status());
    Serial.print(F("RRSI: ")); Serial.println(WiFi.RSSI());
  
  /*
  if(status != WL_CONNECTED){
      log_error("Connecting to wifi because there is no connection to wifi");
  }else{
    return;
  }
  
    int wifi_connection_attempts_count=550;  

  while (wifi_connection_attempts_count-- > 0 ) {
    //if(wifiMulti.run() != WL_CONNECTED){
      
    status = WiFi.waitForConnectResult();
    if(status != WL_CONNECTED){
      log_error((String)"Trying to disconnect from wifi: " + WIFI_SSID);
      WiFi.disconnect();
      log_error((String)"Trying to connect to wifi: " + WIFI_SSID);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      //Serial.print(".");
      delay(100);
//    wifiMulti.disconnect();
//    wifiMulti.reconnect();
    }else{
      log((String )"Connection successfully established. Status digital code: " + WiFi.status() + ". RRSI: " + WiFi.RSSI());
      return;
      break;
    }
  }
  
  if(wifi_connection_attempts_count<=0){
    log_warn("Sleep 30 seconds before restart...");
    delay(30000);
    log_warn("Restarting ESP because connection was lost and couldn't to establish a new one");
    //send_error_code(PR_FORCED_REBOOT);
    ESP.restart();
  }
  */
  }
}

int send_humidity_temperature_to_influxdb(byte* temperature, byte* humidity) {
  if(humidity<=0){
    return -1;
  }
 // Point p_sensor_dth11 = initInfluxDBPoint("dth11");
  p_sensor_dth11.clearFields();
  p_sensor_dth11.addField(F("temperature") , temperature[0]);
  p_sensor_dth11.addField(F("humidity")    , humidity[0]);
  p_sensor_dth11.addField(F("current_HUMIDITY_THRESHOLD_DEFAULT_VALUE")    , currentHumidityThreshold);
  
  log((String)F("Writing dth11: ") + p_sensor_dth11.toLineProtocol());
  
  //if (!influxdbWritePoint(p_sensor_dth11)) {
    if(!client.writePoint(p_sensor_dth11)){
    log_error((String)F("InfluxDB write failed (p_sensor_dth11): ") + client.getLastErrorMessage());
    return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  }
  // if(!client.writePoint(p_current_HUMIDITY_THRESHOLD_DEFAULT_VALUE)){
  //   log_error((String)"InfluxDB write failed (p_current_HUMIDITY_THRESHOLD_DEFAULT_VALUE): " + client.getLastErrorMessage());
  //   return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  // }
  //send_current_HUMIDITY_THRESHOLD_DEFAULT_VALUE(currentHumidityThreshold);

  return PR_SUCCESS;
}



//byte data[40] = {0};
byte temperature = 0;
byte humidity = 0;
boolean isError=false;
boolean firstStart=true;

/*
void checkESPJustStarted(){
  if(firstStart){
    firstStart=false;
    
    p_log.clearFields();
    // p_log.clearTags();
    //p_log.addField("value" , 1);
    // p_log.addField("info",escapeValueSymbolsOfInfluxDBPoint((String)"ESP Started"));
    p_log.addField("info", (String)"ESP Started: " + ESP.getResetReason());
    
    // p_log.addField("reason", ESP.getResetReason());
    //addFieldToPoint(p_log, "info", "ESP Started");
     log((String)"Send log message: " + p_log.toLineProtocol());
    if (!influxdbWritePoint(p_log)) {
        Serial.println("[ERROR] " + (String)"InfluxDB write failed (p_log): " + client.getLastErrorMessage());
    }
  }
}
*/

void update_current_humidity_threshold(){
  if(currentHumidityThreshold==0){
      currentHumidityThreshold=EEPROM.read(EEPROM_ADDR_HUMIDIDITY);
  }else{
    return;
  }  
}

void loop() {
  // checkESPJustStarted();
  update_current_humidity_threshold();
  // DTH11
  //data[40] = {0};
  if (getTH(&temperature, &humidity) < 0) {
    log_error(F("Nothing to send to the InfluxDB because an error was occured on DTH11"));
    //send_error_code(-1);
    isError=true;
    //return;
  }
  log((String)F("pinDHT11 [")+pinDHT11+F("] humidity: [") + humidity+F("]")); //Serial.print(pinDHT11) ; Serial.print(" --> "); Serial.println((int)humidity);



  log((String) F("Current Humidity Threshold: ") + String(currentHumidityThreshold));

  if(checkHumidityExceeded(&humidity)){
    relayCurrentStart();
  }else{
    relayCurrentStop();
  } 


  // webserverUpdateCurrentHumidityValue(int(humidity));
  webserverHandleClient();

  // InfluxDB
  send_humidity_temperature_to_influxdb(&temperature, &humidity);
  // sendMonitoringData();
  //send_error_code(PR_SUCCESSFUL_LOOP_ITERATION);

  wifiConnect();
  
  // Check current value of humidity control against threshold and delta
  if(isRelayWorking()){
    //currentHumidityThreshold=HUMIDITY_THRESHOLD_DEFAULT_VALUE-HUMIDITY_DELTA_FOR_FINISH;
    currentHumidityThreshold=EEPROM.read(EEPROM_ADDR_HUMIDIDITY) - HUMIDITY_DELTA_FOR_FINISH;
  }else{
    currentHumidityThreshold=EEPROM.read(EEPROM_ADDR_HUMIDIDITY);
  }

  //Must be in the loop() to be able access esp8266.local address
  //MDNS.update();
  // if(isError){
  //   log_error((String)"Sleep for 10000 due to error occured");
  //   delay(10000);
  //   isError=false;
  // }
}

/**
 * Parameters: byte* humidity
 * return 0   - Not exceeded
 *       -1   - Exceeded
 */
int checkHumidityExceeded(byte* humidity){
  //unsigned int humidityInt = (/*(humidity[2]<<16)+(humidity[1]<<8)+*/humidity[0]);
  if(humidity[0] /*humidityInt*/>currentHumidityThreshold){
    log_warn((String)F("Humidity [")+humidity[0]+F("] threshold [") + currentHumidityThreshold + F("] was excedeed"));
    return -1;
  }else{
    log((String)F("Humidity [")+humidity[0]+F("] threshold [")+currentHumidityThreshold+F("] was not excedeed"));
  }
  return 0; // Not exceeded
}


int getTH(byte* temperature, byte* humidity) {
  if (millis() - timing > 10000){ // Вместо 10000 подставьте нужное вам значение паузы 
    timing = millis(); 
    log(F("10 seconds elapsed after previous reading DTH11. Let's read once again"));
    byte data[40] = {0};
    if (dht11.read(pinDHT11, temperature, humidity, data)) {
      log_error(F("Read DHT11 failed"));
      return -1;

    // Test Mode
    // log("Test Mode Enabled");
    // temperature=(byte*)20;
    // humidity=(byte*)55;
    // End test mode
    }else{
  //  Serial.print(*temperature); Serial.print(" *C, ");
  //  Serial.print(*humidity); Serial.println(" %");
      return 1;
    }
  }else{
    return 0;
  }
}

/** Stop current flowing
 *  Normally-Open configuration send LOW signal to to let current flow
 *  if you are using normally-closed configuration send HIGH signal
 */
void relayCurrentStop(){
    digitalWrite(RELAY_PIN, LOW);
    log(F("Current not flowing"));
}

/** Start current flowing
 *  Normally-Open configuration send LOW signal to to let current flow
 *  if you are using normally-closed configuration send HIGH signal
 */
void relayCurrentStart(){
  digitalWrite(RELAY_PIN, HIGH);
  log_warn(F("Current flowing..."));
}

boolean relayStatus(){
  log((String)F("Relay status: ") + (digitalRead(RELAY_PIN) == HIGH ? F("working") : F("stopped")));
  return digitalRead(RELAY_PIN);
}

boolean isRelayWorking(){
  if(relayStatus() == HIGH){
    return true;
  }else{
    return false;
  }
}

ADC_MODE(ADC_VCC);


void setup() {
 
  // Setup Serial
  Serial.begin(115200);
  log(F("Setup started"));
  // Setup Relay
  pinMode(RELAY_PIN, OUTPUT);
  relayCurrentStart();
  initWifi();
  wifiConnect();
  //send_error_code(PR_STARTED);
  log(F("Setup of wifi finished"));
  initInfluxDB();
  init_eeprom();
  
  webserver_init();
  
 
  //currentHumidityThreshold=55;//HUMIDITY_THRESHOLD_DEFAULT_VALUE;
}

