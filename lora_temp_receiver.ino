#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "Redmi Note 10S";  // Modify as needed
const char* password = "aditya2708";  // Modify as needed

// ThingSpeak API key
const char* apiKey = "PLFQOEQKUD60QZAB";  // Replace with your ThingSpeak API key

// Define LoRa pins
#define SCK 18
#define MISO 19
#define MOSI 23
#define SS 5
#define RST 15
#define DIO0 26

// Buzzer pin
#define BUZZER_PIN 4 // Define the pin where your buzzer is connected

WebServer server(80);

// Variables to hold the received sensor data
float humidity = 0;
float temperature = 0;
int mq4 = 0;
int mq7 = 0;
int mq135 = 0;
float waterPressure = 0;
int earthquake = 0;

void setup() {
  Serial.begin(9600);

  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  
  // Wait until connected to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Print WiFi connection success and the IP address
  Serial.println();
  Serial.println("Connected to Wi-Fi");
  
  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialization successful");

  // Set up web server routes
  server.on("/", handleRoot);      // Main page
  server.on("/data", handleData);  // JSON data
  server.onNotFound([]() {          // Handle undefined routes
    server.send(404, "text/plain", "404: Not Found");
  });
  server.begin();
  Serial.println("Web server started");

  // Initialize the buzzer pin as output
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Start with the buzzer off
}

void loop() {
  // Check for LoRa packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Packet received
    String receivedData = "";

    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }

    Serial.print("Received: ");
    Serial.println(receivedData);

    // Preprocess the JSON to handle "inf" by replacing it with "0"
    receivedData.replace("inf", "0");

    // Parse the received JSON data
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, receivedData);

    if (error) {
      Serial.print("JSON Parsing failed: ");
      Serial.println(error.c_str());
    } else {
      // Update sensor data from parsed JSON
      humidity = doc["Humidity"];
      temperature = doc["Temperature"];
      mq4 = doc["MQ4"];
      mq7 = doc["MQ7"];
      mq135 = doc["MQ135"];
      waterPressure = doc["WaterPressure"];
      earthquake = doc["Earthquake"];

      // Debug: Print sensor values to check data
      Serial.print("Humidity: "); Serial.println(humidity);
      Serial.print("Temperature: "); Serial.println(temperature);
      Serial.print("MQ4: "); Serial.println(mq4);
      Serial.print("MQ7: "); Serial.println(mq7);
      Serial.print("MQ135: "); Serial.println(mq135);
      Serial.print("Water Pressure: "); Serial.println(waterPressure);
      Serial.print("Earthquake: "); Serial.println(earthquake);

      // Send data to the cloud (ThingSpeak)
      sendToServer();

      // Check if any sensor value exceeds the threshold
      if (humidity > 70 || temperature > 50 || mq4 > 400 || mq7 > 500 || mq135 > 400 || waterPressure > 250 || earthquake > 1000) {
        digitalWrite(BUZZER_PIN, HIGH);  // Turn on the buzzer if any threshold is exceeded
      } else {
        digitalWrite(BUZZER_PIN, LOW);   // Turn off the buzzer otherwise
      }
    }
  }
  
  // Handle client requests
  server.handleClient();
}

// Function to send data to ThingSpeak server
void sendToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Build the ThingSpeak API URL with sanitized data
    String serverUrl = "http://api.thingspeak.com/update?api_key=" + String(apiKey) +
                       "&field1=" + String(humidity) +
                       "&field2=" + String(temperature) +
                       "&field3=" + String(mq4) +
                       "&field4=" + String(mq7) +
                       "&field5=" + String(mq135) +
                       "&field6=" + String(waterPressure) +
                       "&field7=" + String(earthquake);

    // Debug: Print the full URL to check data being sent
    Serial.println("URL: " + serverUrl);

    http.begin(serverUrl);  // Specify the URL

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    // Check for response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Server Response: " + response);
    } else {
      Serial.println("Error on sending GET: " + String(httpResponseCode));
    }

    // End HTTP connection
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

// Function to handle the main web page
void handleRoot() {
  String html = "<html><head><style>";
  html += "body { font-family: Arial, sans-serif; background-color: #e0f7fa; margin: 20px; }"; // Light cyan background
  html += "h1 { color: #00796b; text-align: center; }"; // Dark teal color for heading
  html += "h2 { color: #004d40; }"; // Darker teal for subheadings
  html += "p { font-size: 18px; color: #555; }"; // Gray color for paragraphs
  html += "strong { color: #333; }"; // Dark color for strong text
  html += ".container { max-width: 600px; margin: 0 auto; padding: 20px; border-radius: 10px; background-color: #ffffff; box-shadow: 0 0 10px rgba(0,0,0,0.2); }"; // White container with shadow
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>Received LoRa Sensor Data</h1>";
  html += "<h2>Current Readings:</h2>";
  html += "<p><strong>Humidity:</strong> <span id='humidity'>" + String(humidity) + "%</span></p>";
  html += "<p><strong>Temperature:</strong> <span id='temperature'>" + String(temperature) + " C</span></p>"; // Changed from " °C" to " C"
  html += "<p><strong>MQ-4 (Methane):</strong> <span id='mq4'>" + String(mq4) + " ppm</span></p>";
  html += "<p><strong>MQ-7 (Carbon Monoxide):</strong> <span id='mq7'>" + String(mq7) + " ppm</span></p>";
  html += "<p><strong>MQ-135 (Air Quality):</strong> <span id='mq135'>" + String(mq135) + " ppm</span></p>";
  html += "<p><strong>Water Pressure:</strong> <span id='waterPressure'>" + String(waterPressure) + " Pa</span></p>";
  html += "<p><strong>Earthquake Activity:</strong> <span id='earthquake'>" + String(earthquake) + " units</span></p>";
  html += "</div>";

  // JavaScript for live updates using AJAX every 2 seconds
  html += "<script>";
  html += "setInterval(function() {";
  html += "fetch('/data').then(response => response.json()).then(data => {";
  html += "document.getElementById('humidity').innerHTML = data.humidity + '%';";
  html += "document.getElementById('temperature').innerHTML = data.temperature + ' C';"; // Changed from " °C" to " C"
  html += "document.getElementById('mq4').innerHTML = data.mq4 + ' ppm';";
  html += "document.getElementById('mq7').innerHTML = data.mq7 + ' ppm';";
  html += "document.getElementById('mq135').innerHTML = data.mq135 + ' ppm';";
  html += "document.getElementById('waterPressure').innerHTML = data.waterPressure + ' Pa';";
  html += "document.getElementById('earthquake').innerHTML = data.earthquake + ' units';";
  html += "});";
  html += "}, 2000);"; // Refresh every 2 seconds
  html += "</script>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Function to handle JSON data response
void handleData() {
  StaticJsonDocument<200> doc;
  doc["humidity"] = humidity;
  doc["temperature"] = temperature;
  doc["mq4"] = mq4;
  doc["mq7"] = mq7;
  doc["mq135"] = mq135;
  doc["waterPressure"] = waterPressure;
  doc["earthquake"] = earthquake;

  String response;
  serializeJson(doc, response);

  server.send(200, "application/json", response);
}
