
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

/*
void getTH(byte* _temperature, byte* _humidity) {
  byte data[40] = {0};
  byte temperature;
  byte humidity;
  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    Serial.println("Read DHT11 failed");
    return;
  }
  //  Serial.print("Sample RAW Bits: ");
  //  for (int i = 0; i < 40; i++) {
  //    Serial.print((int)data[i]);
  //    if (i > 0 && ((i + 1) % 4) == 0) {
  //      Serial.print(' ');
  //    }
  //  }
  //  Serial.println("");
  _temperature = &temperature;
  _humidity = &humidity;
  Serial.println("Before");
  Serial.print((int)_temperature); Serial.print(" *C, ");
  Serial.print((int)_humidity); Serial.println(" %");

  //  Serial.print((int)temperature); Serial.print(" *C, ");
  //  Serial.print((int)humidity); Serial.println(" %");

  //  sprintf(lcd_temperature_text, "%d\xDF Temperature", (int)temperature);
  //  sprintf(lcd_humidity_text, "%d%% Humidity",(int)humidity);
}
*/

String ipAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3])  ;
}

void send_humidity_temperature() {
  byte data[40] = {0};
  byte temperature;
  byte humidity;

  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    Serial.println("Read DHT11 failed");
    return;
  }

  InfluxData row_temperature("dth11_temperature");
  row_temperature.addTag("ip", ipAddress2String(WiFi.localIP()));
  row_temperature.addValue("value", (int) temperature);
  influx.write(row_temperature);

  InfluxData row_humidity("dth11_humidity");
  row_humidity.addTag("ip", ipAddress2String(WiFi.localIP()));
  row_humidity.addValue("value", (int) humidity);
  influx.write(row_humidity);

  delay(5000);
}

void loop()
{
  send_humidity_temperature();
  delay(5000);
}
