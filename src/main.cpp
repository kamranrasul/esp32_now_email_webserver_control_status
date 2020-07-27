/*********
  Rui Santos
*********/

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  int id; // must be unique for each sender board
  bool pinStatus[8];
  float temperature;
  float humidity;
  float pressure;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;

// Create an array with all the structures
struct_message boardsStruct = board1;

JSONVar board;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct = myData;
  Serial.println();
  for (int i = 0; i < 8; i++)
  {
    Serial.printf("Control %d is %3s.", i + 1, boardsStruct.pinStatus[i] ? "OFF" : "ON");
    Serial.println();
  }
  Serial.printf("Temperature: %.2f °C or %.2f °F", boardsStruct.temperature, (boardsStruct.temperature * 1.8) + 32);
  Serial.println();
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&boardsStruct, incomingData, sizeof(boardsStruct));

  board["id"] = boardsStruct.id;
  board["temperature"] = boardsStruct.temperature;
  // events.send(jsonString.c_str(), "new_readings", millis());

  Serial.printf("Board ID %u: %u bytes\n", boardsStruct.id, len);
  Serial.printf("Temperature: %.2f °C or %.2f °F", myData.temperature, (myData.temperature * 1.8) + 32);
  Serial.println();

  for (int i = 0; i < 8; i++)
  {
    Serial.printf("Control %d is %3s.", i + 1, boardsStruct.pinStatus[i] ? "OFF" : "ON");
    Serial.println();
  }
}

// setting up esp NOW
void espNowSetup()
{
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}

void setup()
{
  //Initialize Serial Monitor
  Serial.begin(115200);

  // setting up esp NOW
  espNowSetup();
}

void loop()
{
  delay(10000);
}