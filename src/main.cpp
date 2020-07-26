/*********
  Rui Santos
*********/

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Replace with your network credentials
const char *ssid = "Hidden_network";
const char *password = "pak.awan.pk";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// defining the pins need to control and monitor
#define INPIN_1 0
#define INPIN_2 4
#define INPIN_3 12
#define INPIN_4 14
#define INPIN_5 15
#define INPIN_6 26
#define INPIN_7 27
#define INPIN_8 32

// RECEIVER'S MAC Address
uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xF9, 0x0E, 0x98};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message
{
  int id; // must be unique for each sender board
  uint8_t pinStatus[8];
} struct_message;

//Create a struct_message called myData
struct_message myData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// delaying function calls
unsigned long webDelay = millis();
unsigned long espCommDelay = millis();

// reading pin status
void readPinStatus();

// esp to esp communication function
void esp2espComm();

// webserver setup
void esp2Web();

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Setting pin for read mode
  pinMode(INPIN_1, INPUT_PULLUP);
  pinMode(INPIN_2, INPUT_PULLUP);
  pinMode(INPIN_3, INPUT_PULLUP);
  pinMode(INPIN_4, INPUT_PULLUP);
  pinMode(INPIN_5, INPUT_PULLUP);
  pinMode(INPIN_6, INPUT_PULLUP);
  pinMode(INPIN_7, INPUT_PULLUP);
  pinMode(INPIN_8, INPUT_PULLUP);

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

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop()
{
  // reading the PIN status
  readPinStatus();

  if (millis() > espCommDelay)
  {
    esp2espComm();
    espCommDelay = millis() + 10000;
  }
  if (millis() > webDelay)
  {
    esp2Web();
    webDelay = millis() + 5000;
  }
}

void readPinStatus()
{
  // reading the value of PINs
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

void esp2espComm()
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

void esp2Web()
{
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client)
  { // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");

            for (int i = 0; i < 8; i++)
            {
              // writing the Pin State of web

              char pinState[10];
              int num = i + 1;
              itoa(num, pinState, 10);

              // Display current state, and ON/OFF buttons
              String pS = pinState;

              client.println("<p>State of pin: " + pS + "</p>");
              // If the output26State is off, it displays the ON button
              if (myData.pinStatus[i])
              {
                client.println("<p><a href=\"/on\"><button class=\"button\">ON</button></a></p>");
              }
              else
              {
                client.println("<p><a href=\"/off\"><button class=\"button button2\">OFF</button></a></p>");
              }
              Serial.println();

              // The HTTP response ends with another blank line
              client.println();
              // Break out of the while loop
            }
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}