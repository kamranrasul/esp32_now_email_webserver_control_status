/*********
  Rui Santos
*********/

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <string.h>

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

// delaying function calls
unsigned long webDelay = millis();

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

  Serial.printf("Board ID %u: %u bytes", boardsStruct.id, len);
  Serial.println();
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
}

// setting up WiFi
void wifiSetup()
{
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  //while (WiFi.status() != WL_CONNECTED)
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

// webserver setup
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

            // displaying temperature
            char tempVal[10];
            sprintf(tempVal, "%.2f", myData.temperature);
            String temp_Str = tempVal;
            client.println("<p>Temperature: " + temp_Str + "</p>");

            for (int i = 0; i < 8; i++)
            {
              Serial.printf("Control %d is %3s.", i + 1, myData.pinStatus[i] ? "OFF" : "ON");
              Serial.println();

              // writing the Pin State of web
              char pinState[10];
              int num = i + 1;
              itoa(num, pinState, 10);

              // Display current state, and ON/OFF buttons
              String pS = pinState;

              client.println("<p>State of pin: " + pS + "</p>");
              // If the output26State is off, it displays the ON button
              if (!myData.pinStatus[i])
              {
                client.println("<p><a href=\"/on\"><button class=\"button\">ON</button></a></p>");
              }
              else
              {
                client.println("<p><a href=\"/off\"><button class=\"button button2\">OFF</button></a></p>");
              }

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

void setup()
{
  //Initialize Serial Monitor
  Serial.begin(115200);

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

  // setting up WiFi
//  wifiSetup();
}

void loop()
{
/*  if (millis() > webDelay)
  {
//    esp2Web();
    webDelay = millis() + 5000;
  }*/
}