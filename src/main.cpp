/*********
  Rui Santos
*********/

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

// temperature register of MPU-6050
#define TEMP_REG 0X41

// I2C address of the MPU-6050
const int MPU_addr = 0x68;

// variable for storing the register value
int16_t reading;

// defining the pins need to control and monitor
#define INPIN_1 0
#define INPIN_2 2
#define INPIN_3 4
#define INPIN_4 12
#define INPIN_5 14
#define INPIN_6 15
#define INPIN_7 26
#define INPIN_8 27

// RECEIVER'S MAC Address
uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xF9, 0x0E, 0x98};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message
{
  int id; // must be unique for each sender board
  uint8_t pinStatus[8];
  float temperature;
  float humidity;
  float pressure;
} struct_message;

//Create a struct_message called myData
struct_message myData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// setting up pin in input mode
void pinSetup()
{
  // Setting pin for read mode
  pinMode(INPIN_1, INPUT_PULLUP);
  pinMode(INPIN_2, INPUT_PULLUP);
  pinMode(INPIN_3, INPUT_PULLUP);
  pinMode(INPIN_4, INPUT_PULLUP);
  pinMode(INPIN_5, INPUT_PULLUP);
  pinMode(INPIN_6, INPUT_PULLUP);
  pinMode(INPIN_7, INPUT_PULLUP);
  pinMode(INPIN_8, INPUT_PULLUP);
}

// setting up Wire Transfer !2C
void wireSetup()
{
  // I2C setup
  Wire.begin();
  Wire.beginTransmission(MPU_addr);

  // PWR_MGMT_1 register
  Wire.write(0x6B);
  // set to zero (wakes up the MPU-6050)
  Wire.write(0);
  Wire.endTransmission(true);
}

// setting up esp NOW
void espNowSetup()
{
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

// reading Pin Status
void readPins()
{
  // Set values to send
  myData.id = 1;
  myData.pinStatus[0] = digitalRead(INPIN_1);
  myData.pinStatus[1] = digitalRead(INPIN_2);
  myData.pinStatus[2] = digitalRead(INPIN_3);
  myData.pinStatus[3] = digitalRead(INPIN_4);
  myData.pinStatus[4] = digitalRead(INPIN_5);
  myData.pinStatus[5] = digitalRead(INPIN_6);
  myData.pinStatus[6] = digitalRead(INPIN_7);
  myData.pinStatus[7] = digitalRead(INPIN_8);
}

// esp NOW send message
void espNowSend()
{
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK)
  {
    Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }
}

// reading from MPU
void mpu_read() {
  // setting the controller for read
  Wire.beginTransmission(MPU_addr);

  // setting for register for reading the value
  Wire.write(TEMP_REG);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 2, true);

  // reading 2 registers 0x41 and 0x42, as high and low
  reading = Wire.read() << 8 | Wire.read();

  // converting to from raw to engineering value
  myData.temperature = reading / 340.0 + 36.53;

  // displaying on the serial output
  Serial.printf("Temperature: %.2f °C or %.2f °F", myData.temperature, (myData.temperature * 1.8) + 32);
  Serial.println();
}

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);

  pinSetup();
  wireSetup();
  espNowSetup();
}

void loop()
{
  // reading Pins
  readPins();

  // reading from MPU
  mpu_read();

  // sending message
  espNowSend();

  delay(10000);
}
