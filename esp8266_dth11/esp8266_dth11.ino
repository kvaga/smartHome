
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <InfluxDb.h>
#include <SimpleDHT.h>
#include <Wire.h>
#include <credentials.h>

int pinDHT11 = 14;
SimpleDHT11 dht11;
char lcd_temperature_text[256];
char lcd_humidity_text[256];

Influxdb influx(INFLUXDB_HOST);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    //    Serial.print(WiFi.status());
  }
  Serial.println(" connected");

  influx.setDb(INFLUXDB_DB);
  influx.setPort(INFLUXDB_PORT);
  // Uncomment if you want to use InfluxDB v2
  //  influx.setBucket(“myBucket");
  //  influx.setVersion(2);
  //  influx.setOrg(“myOrg");
  //  influx.setToken(“myToken");
}


int getTH(byte* temperature, byte* humidity) {
  byte data[40] = {0};
  if (dht11.read(pinDHT11, temperature, humidity, data)) {
    Serial.println("Read DHT11 failed");
    return -1;
  }
//  Serial.print(*temperature); Serial.print(" *C, ");
//  Serial.print(*humidity); Serial.println(" %");
  return 0;
}


String ipAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3])  ;
}

int send_humidity_temperature() {
  byte data[40] = {0};
  byte temperature=0;
  byte humidity=0;

  if(getTH(&temperature,&humidity)<0){
    Serial.println("Nothing to send to the InfluxDB because an error was occured on DTH11");
    return -1;
  }

  InfluxData row_temperature("dth11_temperature");
  row_temperature.addTag("ip", ipAddress2String(WiFi.localIP()));
  row_temperature.addValue("value", (int) temperature);
  influx.write(row_temperature);

  InfluxData row_humidity("dth11_humidity");
  row_humidity.addTag("ip", ipAddress2String(WiFi.localIP()));
  row_humidity.addValue("value", (int) humidity);
  influx.write(row_humidity);

//  delay(5000);
}

void loop()
{
  send_humidity_temperature();
}
