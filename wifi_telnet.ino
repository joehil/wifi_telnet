#include <ESP8266WiFi.h>
#include "WebOTA.h"

const char* ssid = "<ssid>";
const char* password = "<password>";
const int   baudrate = 115200;

//Static IP address configuration
IPAddress staticIP(192, 168, 0, 157); //ESP static ip
IPAddress gateway(192, 168, 0, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);  //DNS

const char* deviceName = "wifi_telnet";
const char* sversion = "1.0.1";

uint8_t i;
uint32_t cnt_unconn=0;
bool ConnectionEstablished; // Flag for successfully handled connection
char modus = 'c';

WiFiServer TelnetServer(11023);
WiFiClient TelnetClient;

void setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(staticIP, gateway, subnet, dns);
  // ... Give ESP 10 seconds to connect to station.
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
  {
    delay(200);
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(3000);
    ESP.restart();
  }
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);
}

void loop() {
  webota.handle();
  Telnet();  // Handle telnet connections
  if (WiFi.status() != WL_CONNECTED) {
    cnt_unconn++;
  } else {
    cnt_unconn = 0;
  }
  if (cnt_unconn > 100000) {
    delay(3000);
    ESP.restart();
  }
}

void Telnet()
{ if (!TelnetClient)     cnt_unconn++;
  
  if (TelnetClient && !TelnetClient.connected())
  {
    TelnetClient.stop();
    cnt_unconn++;
  }

  // Check new client connections
  if (TelnetServer.hasClient())
  {
    ConnectionEstablished = false; // Set to false

    if (!TelnetClient)
    {
      TelnetClient = TelnetServer.available();

      Serial.begin(baudrate);

      modus = 'c';

      TelnetClient.flush();  // clear input buffer, else you get strange characters
      TelnetClient.println("Welcome!");

      TelnetClient.print("wifi_telnet version ");
      TelnetClient.println(sversion);

      TelnetClient.print("Millis since start: ");
      TelnetClient.println(millis());

      TelnetClient.print("Free Heap RAM: ");
      TelnetClient.println(ESP.getFreeHeap());

      TelnetClient.print("Baudrate: ");
      TelnetClient.println(baudrate);

      TelnetClient.print("Modus: ");
      TelnetClient.println(modus);

      TelnetClient.println("ready");

      ConnectionEstablished = true;
    }

    if (ConnectionEstablished == false)
    {
      TelnetServer.available().stop();
    }
  }

  if (TelnetClient && TelnetClient.connected())
  {
    if (TelnetClient.available())
    {
      //get data from the telnet client
      while (TelnetClient.available())
      {
        char c = TelnetClient.read();
        if (modus == 'c') {
          if (c == 'a') {
            Serial.write("\x02\x0e\x01\x04\x51\x03");
            TelnetClient.write("auto\r\n");
          }
          if (c == 'h') {
            Serial.write("\x02\x0e\x01\x02\x8c\x03");
            TelnetClient.write("home\r\n");
          }
          if (c == 'm') {
            Serial.write("\x02\x0e\x01\x03\xd2\x03");
            TelnetClient.write("manuell\r\n");
          }
          if (c == 'u') {
            TelnetClient.print("Millis since start: ");
            TelnetClient.println(millis());
            TelnetClient.print("Modus: ");
            TelnetClient.println(modus);
          }
          if (c == 'z') {
            Serial.write("\x02\x02\x04\x80\x0c\x28\x00\xa4\x03");
            TelnetClient.write("zeit\r\n");
          }
          if (c == 'x') {
            modus = 'x';
            TelnetClient.write("Modus auf x gesetzt\r\n");
          }
        }
        if (modus == 'x') {
          Serial.write(c);
        }
      }
    }
  }
  if (Serial.available() > 0)
  {
    //get data from Serial
    while (Serial.available() > 0)
    {
      char c = Serial.read();
      TelnetClient.write(c);
    }
  }
  delay(10);
}
