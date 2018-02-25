
#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"

//PIN CONFIGURATION
#define TEMPSENSORPIN 40

//SENSOR CONFIGURATION
#define DHTTYPE DHT22

//MAC
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//NETWORK SETTINGS
IPAddress ip(192, 168, 1, 10);
IPAddress dnsserver(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

//SERVER SETTINGS
IPAddress serverIP(192, 168, 1, 2);
char serverName[] = "monitor.yourserver";
int serverPort = 80;

//ETHERNET CLIENT
EthernetClient client;

//ULR PARAMS
char params[32];
char pageName[] = "/your/reading/api";

//SENSOR SETTINGS
DHT dht(TEMPSENSORPIN, DHTTYPE);
float tempF = 0.0;
float humidF = 0.0;
String temp = "0.0";
String humid = "0.0";

void setup() {
  Serial.begin(9600);

  // disable SD SPI
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  // Start ethernet
  Serial.println("STARTING ETHERNET");
  Ethernet.begin(mac, ip, dnsserver, gateway, subnet);

  Serial.println(Ethernet.localIP());

  delay(2000);
  Serial.println(F("ETHERNET READY"));
}

void loop() {
    tempHumidReading();
    sprintf(params,"location=%i&temp=%s&humid=%s", 1, temp.c_str(), humid.c_str());
 
    Serial.println(params);
    
    if(!postData(serverName,serverPort,pageName,params)) {
      Serial.print("DATA FAILED");
    }
    else { 
      Serial.println("DATA POSTED");
    }
  
    delay(60000);
}

//temp humid reading
void tempHumidReading() {
    humidF = dht.readHumidity();
    tempF = dht.readTemperature();
    if(!isnan(humidF)) {
        String humidStr = "";
        char hchar[5];
        dtostrf(humidF, 2, 2, hchar);
        humidStr = (String)hchar;
        humidStr.trim();
        humid = humidStr;
    }
    else {
        humid = "0";
    }
    if(!isnan(tempF)) {
        String tempStr = "";
        char tchar[5];
        dtostrf(tempF, 2, 2, tchar);
        tempStr = (String)tchar;
        tempStr.trim();
        temp = tempStr;
    }
    else {
        temp = "0";
    }
}

//post data to server
byte postData(char* sv, int port, char* page, char* data)
{
  int inChar;
  char outBuf[64];

  Serial.print("CONNECTING TO SERVER... ");

  if(client.connect(sv, port) == 1)
  {
    Serial.println("CONNECTED");

    //send header
    sprintf(outBuf,"POST %s HTTP/1.1",page);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s",serverName);
    client.println(outBuf);
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(data));
    client.println(outBuf);

    //send body data
    client.print(data);
  } 
  else
  {
    Serial.println("CONNECTION FAILED");
    return 0;
  }

  int connectLoop = 0;

  while(client.connected())
  {
    while(client.available())
    {
      inChar = client.read();
      Serial.write(inChar);
      connectLoop = 0;
    }

    delay(1);
    connectLoop++;
    if(connectLoop > 10000)
    {
      Serial.println();
      Serial.println(F("TIMEOUT"));
      client.stop();
    }
  }

  Serial.println();
  Serial.println(F("DISCONNECTING..."));
  client.stop();
  return 1;
}
