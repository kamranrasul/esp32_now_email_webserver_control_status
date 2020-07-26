#include <Arduino.h>
#include <WiFi.h>

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  // extracting the mac address and sending it to serial port
  Serial.println(WiFi.macAddress());
}

void loop()
{
}