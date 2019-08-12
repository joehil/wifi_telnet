#include <ESP8266WiFi.h>

const char* ssid = "<ssid>";
const char* password = "<password>";

//Static IP address configuration
IPAddress staticIP(192, 168, 0, 156); //ESP static ip
IPAddress gateway(192, 168, 0, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);  //DNS


const char* deviceName = "wifi_telnet";

uint8_t i;
bool ConnectionEstablished; // Flag for successfully handled connection

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
  Telnet();  // Handle telnet connections
}

void TelnetMsg(String text)
{
  if (TelnetClient || TelnetClient.connected())
  {
    TelnetClient.println(text);
  }
  delay(10);  // to avoid strange characters left in buffer
}

void Telnet()
{ if (TelnetClient && !TelnetClient.connected())
  {
    TelnetClient.stop();
  }

  // Check new client connections
  if (TelnetServer.hasClient())
  {
    ConnectionEstablished = false; // Set to false

    if (!TelnetClient)
    {
      TelnetClient = TelnetServer.available();

      Serial.begin(9600);

      TelnetClient.flush();  // clear input buffer, else you get strange characters
      TelnetClient.println("Welcome!");

      TelnetClient.print("Millis since start: ");
      TelnetClient.println(millis());

      TelnetClient.print("Free Heap RAM: ");
      TelnetClient.println(ESP.getFreeHeap());

      TelnetClient.println("ready");

      ConnectionEstablished = true;
    }

    if (ConnectionEstablished == false)
    {
      TelnetServer.available().stop();
      // TelnetMsg("An other user cannot connect ... MAX_TELNET_CLIENTS limit is reached!");
    }
  }

  if (TelnetClient && TelnetClient.connected())
  {
    if (TelnetClient.available())
    {
      //get data from the telnet client
      while (TelnetClient.available())
      {
        Serial.write(TelnetClient.read());
      }
    }
    if (Serial.available())
    {
      //get data from Serial
      while (Serial.available())
      {
        TelnetClient.write(Serial.read());
      }
    }
  }
}
