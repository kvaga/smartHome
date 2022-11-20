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

#include "lib_log.h"
#include <credentials.h>
#include "constants.h"
#include "InfluxDbClient.h"
#include "InfluxDbCloud.h"
#include <SoftwareSerial.h>;
//#include <MemoryFree.h>;

#if defined(ESP32)
    #include <WiFiMulti.h>
    WiFiMulti wifiMulti;
    #define DEVICE "ESP32"
#elif defined(ESP8266)
//    #include <ESP8266WiFiMulti.h>
//    ESP8266WiFiMulti wifiMulti;
    #include <ESP8266WiFi.h>
    #include <WiFiClient.h>
    #include <ESP8266WebServer.h>
    #include <ESP8266mDNS.h>
    #include "webserver.h"
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

const String HOSTNAME="bath-humidity";
unsigned long timing;
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// InfluxDB data point
// Point sensor("wifi_status");
Point sensor_dth11("dth11");
Point error_code("error_code");
Point p_free_heap("free_heap");
Point p_current_humidity_threshold("current_humidity_threshold");
#include <SimpleDHT.h>
SimpleDHT11 dht11;

int currentHumidityThreshold=55;//HUMIDITY_THRESHOLD;

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
const int pinDHT11 = 14; // пин data сенсора dth14 к D5 esp8266
// void log(String str){
//   Serial.println("[INFO] " + str);
// }
// void log_warn(String str){
//   Serial.println("[WARN] " + str);
// }
// void log_error(String str){
//   Serial.println("[ERROR] " + str);
// }

void timeSync() {
  // Synchronize UTC time with NTP servers
  // Accurate time is necessary for certificate validaton and writing in batches
  configTime(0, 0, "pool.ntp.org", "time.nis.gov");
  // Set timezone
  setenv("TZ", TZ_INFO, 1);

  // Wait till time is synced
  log("Syncing time");
  int i = 0;
  while (time(nullptr) < 1000000000ul && i < 100) {
    Serial.print(".");
    delay(100);
    i++;
  }
  log("");
  // Show time
  time_t tnow = time(nullptr);
  log((String)"Synchronized time: " + String(ctime(&tnow)));
}

String ipAddress2String(const IPAddress& ipAddress)
{
    return  String(ipAddress[0]) + String(".") + \
            String(ipAddress[1]) + String(".") + \
            String(ipAddress[2]) + String(".") + \
            String(ipAddress[3])  ;
}

void initWifi(){
  log("Init of wifi started");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.hostname(HOSTNAME.c_str());
  //wifiConnect();
  //wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  log((String)"Connected to wifi: " + WiFi.SSID() +  ", hostname=" + WiFi.hostname().c_str());
  log((String)"ip=" + WiFi.localIP().toString());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  log("Init of wifi stoped");
}

void initInfluxDB(){
  log("Init of influXDB started");
  // Setup InfluXDB tags
  sensor_dth11.addTag("device", DEVICE);
  sensor_dth11.addTag("SSID", WiFi.SSID());
  sensor_dth11.addTag("ip", ipAddress2String(WiFi.localIP()));
  sensor_dth11.addTag("host",WiFi.hostname().c_str());
  sensor_dth11.addTag("localtion","home2");

  error_code.addTag("device", DEVICE);
  error_code.addTag("SSID", WiFi.SSID());
  error_code.addTag("ip", ipAddress2String(WiFi.localIP()));
  error_code.addTag("host",WiFi.hostname().c_str());
  error_code.addTag("location","home2");

  p_free_heap.addTag("device", DEVICE);
  p_free_heap.addTag("SSID", WiFi.SSID());
  p_free_heap.addTag("ip", ipAddress2String(WiFi.localIP()));
  p_free_heap.addTag("host",WiFi.hostname().c_str());
  p_free_heap.addTag("location","home2");

  
  p_current_humidity_threshold.addTag("device", DEVICE);
  p_current_humidity_threshold.addTag("SSID", WiFi.SSID());
  p_current_humidity_threshold.addTag("ip", ipAddress2String(WiFi.localIP()));
  p_current_humidity_threshold.addTag("host",WiFi.hostname().c_str());
  p_current_humidity_threshold.addTag("location","home2");
  // Sync time for certificate validation
  timeSync();

  // Check server connection
  if (client.validateConnection()) {
    log((String)"Connected to InfluxDB: " + client.getServerUrl());
    //Get Current Hostname
  } else {
    log_error((String)"InfluxDB connection failed: " + client.getLastErrorMessage());
  }
  log("Init of influxDB finished");
}

void setup() {
  // Setup Serial
  Serial.begin(115200);
  log("Setup started");

  // Setup Relay
  pinMode(RELAY_PIN, OUTPUT);
  relayCurrentStart();
  initWifi();
  wifiConnect();
 
  //send_error_code(PR_STARTED);
  log("Setup of wifi finished");
  initInfluxDB();
  webserver_init();
  int EEPROM_HUMIDIDITY=0;
 
  //currentHumidityThreshold=55;//HUMIDITY_THRESHOLD;
}

void wifiConnect(){
  int wifi_connection_attempts_count=550;  
//  if(wifiMulti.run() != WL_CONNECTED){
    uint8_t status = WiFi.waitForConnectResult();
    switch (status){
      case WL_NO_SSID_AVAIL:
        log("Connection status text: configured SSID cannot be reached"); break;
      case WL_CONNECTED:
        log("Connection status text: connection successfully established");
        return;
        break;
      case WL_CONNECT_FAILED:
        Serial.print("Connection status text: connection failed");
        break;
      case WL_NO_SHIELD:
        Serial.print("Connection status text: WL_NO_SHIELD"); break;  
      case WL_IDLE_STATUS:
        Serial.print("Connection status text: WL_IDLE_STATUS"); break;
      case WL_SCAN_COMPLETED:
        Serial.print("Connection status text: WL_SCAN_COMPLETED"); break;
      case WL_DISCONNECTED:
        Serial.print("Connection status text: WL_DISCONNECTED"); break;
      case WL_CONNECTION_LOST:
        Serial.print("Connection status text: WL_CONNECTION_LOST"); break;
    }
    Serial.println();
    Serial.printf("Connection status digital code: %d\n", WiFi.status());
    Serial.print("RRSI: "); Serial.println(WiFi.RSSI());
  if(status != WL_CONNECTED){
      log_error("Connecting to wifi because there is no connection to wifi");
  }else{
    return;
  }
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
}

int send_memory_metrics(){
  //F function does the same and is now a built in library, in IDE > 1.0.0
   Serial.println(F("Free RAM = "));
  long  fh = ESP.getFreeHeap();
  char  fhc[20];

  ltoa(fh, fhc, 10);
  Serial.println(fh);
  p_free_heap.clearFields();
  p_free_heap.addField("bytes", fh);
  log((String)"Writing p_free_heap: " + p_free_heap.toLineProtocol());

  // If no Wifi signal, try to reconnect it
//  if (wifi_connection_lost()){
//    return -1;
//  }
  // Write point
  if (!client.writePoint(p_free_heap)) {
    log_error((String)"InfluxDB write failed (p_free_heap): " + client.getLastErrorMessage());
    return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  }
   // print how much RAM is available.
//   Serial.println(ramfree(), DEC);
//      Serial.println(flashfree(), DEC);
   // print how much flash is available.
}

int send_humidity_temperature_to_influxdb(byte* temperature, byte* humidity) {
  sensor_dth11.clearFields();
  sensor_dth11.addField("temperature" , temperature[0]);
  sensor_dth11.addField("humidity"    , humidity[0]);
  
  log((String)"Writing dth11: " + sensor_dth11.toLineProtocol());
  
  if (!client.writePoint(sensor_dth11)) {
    log_error((String)"InfluxDB write failed (dth11): " + client.getLastErrorMessage());
    return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  }
  return PR_SUCCESS;
}

int send_error_code(int errorCode){
    error_code.clearFields();
    error_code.addField("_code" , errorCode);
  log((String)"Writing error code: " + error_code.toLineProtocol());
  
  if (!client.writePoint(error_code)) {
    log_error((String)"InfluxDB write failed (error_code): " + client.getLastErrorMessage());
    return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  }
  return PR_SUCCESS;
}

int send_current_humidity_threshold(int currentHumidityThreshold){
  p_current_humidity_threshold.clearFields();
  p_current_humidity_threshold.addField("value" , currentHumidityThreshold);
  log((String)"Writing error code: " + p_current_humidity_threshold.toLineProtocol());
  if (!client.writePoint(p_current_humidity_threshold)) {
    log_error((String)"InfluxDB write failed (error_code): " + client.getLastErrorMessage());
    return PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER;
  }
  return PR_SUCCESS;
}

//byte data[40] = {0};
byte temperature = 0;
byte humidity = 0;
boolean isError=false; 

void loop() {
  if(read(EEPROM_HUMIDIDITY)){
    currentHumidityThreshold=read(EEPROM_HUMIDIDITY);
  }else{
    currentHumidityThreshold=HUMIDITY_THRESHOLD;
    save(EEPROM_HUMIDIDITY, currentHumidityThreshold);
  }

  // DTH11
  //data[40] = {0};
  if (getTH(&temperature, &humidity) < 0) {
    log_error("Nothing to send to the InfluxDB because an error was occured on DTH11");
    //send_error_code(-1);
    isError=true;
    //return;
  }
  log((String)"pinDHT11 ["+pinDHT11+"] humidity: [" + humidity+"]"); //Serial.print(pinDHT11) ; Serial.print(" --> "); Serial.println((int)humidity);



  log((String) "Current Humidity Threshold: " + String(currentHumidityThreshold));

  if(checkHumidityExceeded(&humidity)){
    relayCurrentStart();
  }else{
    relayCurrentStop();
  } 

  webserverUpdateCurrentHumidityValue(int(humidity));
  webserverHandleClient();
  
  // InfluxDB
  send_humidity_temperature_to_influxdb(&temperature, &humidity);
  send_memory_metrics();
  //send_error_code(PR_SUCCESSFUL_LOOP_ITERATION);

  wifiConnect();
  
  // Check current value of humidity control against threshold and delta
  if(isRelayWorking()){
    //currentHumidityThreshold=HUMIDITY_THRESHOLD-HUMIDITY_DELTA_FOR_FINISH;
    currentHumidityThreshold=(read(EEPROM_HUMIDIDITY)?read(EEPROM_HUMIDIDITY):HUMIDITY_THRESHOLD) - HUMIDITY_DELTA_FOR_FINISH;
  }else{
    currentHumidityThreshold=read(EEPROM_HUMIDIDITY)?read(EEPROM_HUMIDIDITY):HUMIDITY_THRESHOLD;
  }

  send_current_humidity_threshold(currentHumidityThreshold);
  if(isError){
    log_error((String)"Sleep for 10000 due to error occured");
    delay(10000);
    isError=false;
  }
}

/**
 * Parameters: byte* humidity
 * return 0   - Not exceeded
 *       -1   - Exceeded
 */
int checkHumidityExceeded(byte* humidity){
  unsigned int humidityInt = (/*(humidity[2]<<16)+(humidity[1]<<8)+*/humidity[0]);
  if(humidityInt>currentHumidityThreshold){
    log_warn((String)"Humidity ["+humidityInt+"] threshold [" + currentHumidityThreshold + "] was excedeed");
    return -1;
  }else{
    log((String)"Humidity ["+humidityInt+"] threshold ["+currentHumidityThreshold+"] was not excedeed");
  }
  return 0; // Not exceeded
}


int getTH(byte* temperature, byte* humidity) {
  if (millis() - timing > 10000){ // Вместо 10000 подставьте нужное вам значение паузы 
    timing = millis(); 
    log("10 seconds elapsed after previous reading DTH11. Let's read once again");
    byte data[40] = {0};
    if (dht11.read(pinDHT11, temperature, humidity, data)) {
      log_error("Read DHT11 failed");
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
    log("Current not flowing");
}

/** Start current flowing
 *  Normally-Open configuration send LOW signal to to let current flow
 *  if you are using normally-closed configuration send HIGH signal
 */
void relayCurrentStart(){
  digitalWrite(RELAY_PIN, HIGH);
  log_warn("Current flowing...");
}

boolean relayStatus(){
  log((String)"Relay status: " + (digitalRead(RELAY_PIN) == HIGH ? "working" : "stopped"));
  return digitalRead(RELAY_PIN);
}

boolean isRelayWorking(){
  if(relayStatus() == HIGH){
    return true;
  }else{
    return false;
  }
}
