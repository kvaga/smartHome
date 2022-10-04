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

#include <credentials.h>
#include "InfluxDbClient.h"
#include "InfluxDbCloud.h"
#include <SoftwareSerial.h>;
//#include <MemoryFree.h>;

#if defined(ESP32)
    #include <WiFiMulti.h>
    WiFiMulti wifiMulti;
    #define DEVICE "ESP32"
#elif defined(ESP8266)
    #include <ESP8266WiFiMulti.h>
    ESP8266WiFiMulti wifiMulti;
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
// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// InfluxDB data point
// Point sensor("wifi_status");
Point sensor_dth11("dth11");
Point error_code("error_code");
Point p_free_heap("free_heap");

#include <SimpleDHT.h>
SimpleDHT11 dht11;


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


void setup() {
  // Setup Serial
  Serial.begin(115200);

  // Setup Relay
  pinMode(RELAY_PIN, OUTPUT);
  relayCurrentStart();
  // Setup wifi
  WiFi.mode(WIFI_STA);
  //Set new hostname
  WiFi.hostname(HOSTNAME.c_str());

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  wifiConnect();
  log("Hostname: " + WiFi.hostname());

  /*
  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
*/
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

  // Sync time for certificate validation
  timeSync();

  // Check server connection
  if (client.validateConnection()) {
    log((String)"Connected to InfluxDB: " + client.getServerUrl());
    //Get Current Hostname
  } else {
    log_error((String)"InfluxDB connection failed: " + client.getLastErrorMessage());
  }
}

void wifiConnect(){
  int wifi_connection_attempts_count=150;  
  Serial.print("\nConnecting to wifi");
  while (wifi_connection_attempts_count-- > 0 && wifiMulti.run() != WL_CONNECTED ) {
    Serial.print(".");
    delay(100);
//    wifiMulti.disconnect();
//    wifiMulti.reconnect();
  }
  Serial.println();

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
  if (wifi_connection_lost()){
    return -1;
  }
  // Write point
  if (!client.writePoint(p_free_heap)) {
    log_error((String)"InfluxDB write failed (p_free_heap): " + client.getLastErrorMessage());
    return -2;
  }
   // print how much RAM is available.
//   Serial.println(ramfree(), DEC);
//      Serial.println(flashfree(), DEC);
   // print how much flash is available.
}
void timeSync() {
  // Synchronize UTC time with NTP servers
  // Accurate time is necessary for certificate validaton and writing in batches
  configTime(0, 0, "pool.ntp.org", "time.nis.gov");
  // Set timezone
  setenv("TZ", TZ_INFO, 1);

  // Wait till time is synced
  Serial.print("Syncing time");
  int i = 0;
  while (time(nullptr) < 1000000000ul && i < 100) {
    Serial.print(".");
    delay(100);
    i++;
  }
  Serial.println();

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

int wifi_connection_lost(){
  if(/*WiFi.RSSI() == 0 && */ wifiMulti.run() != WL_CONNECTED){
    log_error((String)"Wifi connection lost");
    return 1;
  }else{
    return 0;
  }
}
int send_humidity_temperature_to_influxdb(byte* temperature, byte* humidity) {
  sensor_dth11.clearFields();
  sensor_dth11.addField("temperature" , temperature[0]);
  sensor_dth11.addField("humidity"    , humidity[0]);
  
  log((String)"Writing dth11: " + sensor_dth11.toLineProtocol());
  
  // If no Wifi signal, try to reconnect it
  if (wifi_connection_lost()){
    return -1;
  }
  // Write point
  if (!client.writePoint(sensor_dth11)) {
    log_error((String)"InfluxDB write failed (dth11): " + client.getLastErrorMessage());
    return -2;
  }
  return 0;
}

int send_error_code(int errorCode){
    error_code.clearFields();
    error_code.addField("code" , errorCode);
  log((String)"Writing error code: " + error_code.toLineProtocol());
  
  // If no Wifi signal, try to reconnect it
  if (wifi_connection_lost()){
    return -1;
  }
  // Write point
  if (!client.writePoint(error_code)) {
    log_error((String)"InfluxDB write failed (error_code): " + client.getLastErrorMessage());
    return -2;
  }
  return 0;
}

void loop() {
  // DTH11
  byte data[40] = {0};
  byte temperature = 0;
  byte humidity = 0;


  if (getTH(&temperature, &humidity) < 0) {
    log_error("Nothing to send to the InfluxDB because an error was occured on DTH11");
    if(send_error_code(-1)==-1){
      wifiConnect();
    }
    delay(30000);
    return;
  }
  log((String)"pinDHT11 ["+pinDHT11+"] humidity: [" + humidity+"]"); //Serial.print(pinDHT11) ; Serial.print(" --> "); Serial.println((int)humidity);

  if(checkHumidityExceeded(&humidity)){
    relayCurrentStart();
  }else{
    relayCurrentStop();
  } 

  // InfluxDB
  if(send_humidity_temperature_to_influxdb(&temperature, &humidity)==-1){
    wifiConnect();
  }

   send_memory_metrics();

  // Wait
  log("Wait 10s");
  delay(10000);
}

/**
 * Parameters: byte* humidity
 * return 0   - Not exceeded
 *       -1   - Exceeded
 */
int checkHumidityExceeded(byte* humidity){
  unsigned int humidityInt = (/*(humidity[2]<<16)+(humidity[1]<<8)+*/humidity[0]);
  int HUMIDITY_THRESHOLD = 55;
  if(humidityInt>HUMIDITY_THRESHOLD){
    log_warn((String)"Humidity ["+humidityInt+"] threshold [" + HUMIDITY_THRESHOLD + "] was excedeed");
    return -1;
  }else{
    log((String)"Humidity ["+humidityInt+"] threshold ["+HUMIDITY_THRESHOLD+"] was not excedeed");
  }
  return 0; // Not exceeded
}

void log(String str){
  Serial.println("[INFO] " + str);
}
void log_warn(String str){
  Serial.println("[WARN] " + str);
}

void log_error(String str){
  Serial.println("[ERROR] " + str);
}

int getTH(byte* temperature, byte* humidity) {
  byte data[40] = {0};
  if (dht11.read(pinDHT11, temperature, humidity, data)) {
    log_error("Read DHT11 failed");
    return -1;
  }
  //  Serial.print(*temperature); Serial.print(" *C, ");
  //  Serial.print(*humidity); Serial.println(" %");
  return 0;
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
