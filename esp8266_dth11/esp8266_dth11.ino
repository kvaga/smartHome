
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <InfluxDb.h>


#include <SimpleDHT.h>
#include <Wire.h>

int pinDHT11 = 14;
SimpleDHT11 dht11;
char lcd_temperature_text[256];
char lcd_humidity_text[256];
////
const char* ssid="***";
const char* password="*****";

//const char* ssid="*****";
//const char* password="******";
Influxdb influx("");

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");
//  byte t;
//  byte h;
//  getTH(&t,&h);
//   Serial.println("after");
//  Serial.print(t); Serial.print(" *C, ");
//  Serial.print(h); Serial.println(" %");

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
//    Serial.print(WiFi.status());
  }
  Serial.println(" connected");
  
  influx.setDb("home1");
  influx.setPort(8086);
    // Uncomment if you want to use InfluxDB v2
//  influx.setBucket(“myBucket");
//  influx.setVersion(2);
//  influx.setOrg(“myOrg");
//  influx.setToken(“myToken");
}

void getTH(byte* _temperature, byte* _humidity){
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
  _temperature=&temperature;
  _humidity=&humidity;
  Serial.println("Before");
  Serial.print((int)_temperature); Serial.print(" *C, ");
  Serial.print((int)_humidity); Serial.println(" %");
  
//  Serial.print((int)temperature); Serial.print(" *C, ");
//  Serial.print((int)humidity); Serial.println(" %");
  
//  sprintf(lcd_temperature_text, "%d\xDF Temperature", (int)temperature);
//  sprintf(lcd_humidity_text, "%d%% Humidity",(int)humidity);
}

String ipAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}

void f2(){
  byte data[40] = {0};
  byte temperature;
  byte humidity;
  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    Serial.println("Read DHT11 failed");
    return;
  }
  

    InfluxData row_temperature("dth11_temperature");
//    row_temperature.addTag("ip", ipAddress2String(WiFi.localIP()));
    row_temperature.addTag("ip", ipAddress2String(WiFi.localIP()));
    row_temperature.addValue("value", (int) temperature);
//    row_temperature.addValue("value", 32);
    influx.write(row_temperature);
    
    InfluxData row_humidity("dth11_humidity");
    row_humidity.addTag("ip", ipAddress2String(WiFi.localIP()));
    row_humidity.addValue("value", (int) humidity);
    influx.write(row_humidity);

    
    delay(5000);
}

void f1(){
  if(WiFi.status()== WL_CONNECTED){ 
  WiFiClient client;
  byte temperature;
  byte humidity;
//  getTH(temperature, humidity);
  byte data[40] = {0};
  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    Serial.println("Read DHT11 failed");
    return;
  }
  Serial.print((int)temperature); Serial.print(" *C, ");
  Serial.print((int)humidity); Serial.println(" %");
  IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    HTTPClient http;
   http.begin("http://:8086/write?db=home1");      //Specify request destination
   http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//   http.setAuthorization(influxUser, influxPassword);
   int httpCode = -1;
   Serial.print("Sending data ...");
   while(httpCode == -1){
    Serial.print(".");
    httpCode = http.POST("dth11_temperature,ip=192.168.0.0.23 value=25\n");   //Send the request
    http.writeToStream(&Serial);
   }
   Serial.println();
   Serial.println("The data was sent");
   String payload = http.getString();
   Serial.print("httpCode="); Serial.println(httpCode);   //Print HTTP return code
   Serial.print("Response="); Serial.println(payload);    //Print request response payload
 
   http.end();  //Close connection
  }else{
    Serial.println("Error in WiFi connection");   
 
  }
  
}
void loop()
{
  f2();
  /*
  Serial.printf("\n[Connecting to %s ... ", host);
  if (1 || client.connect(host, 8086))
  {
    Serial.println("connected]");
   
  
  
    Serial.println("[Sending a request]");
//    client.print(String("GET /") + " HTTP/1.1\r\n" +
//                 "Host: " + host + "\r\n" +
//                 "Connection: close\r\n" +
//                 "\r\n"
//                );
    
    

    Serial.println("[Response:]");
    while (client.connected() || client.available())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    client.stop();
    Serial.println("\n[Disconnected]");
  }
  else
  {
    Serial.println("connection failed!]");
    client.stop();
  }
  */
  delay(5000);
}
