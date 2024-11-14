#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"

// Pin definitions
#define BUZZER_PIN 3
#define DHTPIN A3
#define DHTTYPE DHT11
#define x A5
#define input A4

// Constants
#define SAMPLES 50
int xsample = 0;

// Variables for sensor readings
DHT dht(DHTPIN, DHTTYPE);
int X, Y;
float TIME = 0;
float FREQUENCY = 0;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
  dht.begin();

  // Initialize LoRa
  Serial.println("LoRa Sender");
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Taking samples for calibration
  for (int i = 0; i < SAMPLES; i++) {
    xsample += analogRead(x);
  }
  xsample /= SAMPLES;
}

void loop() {
  // Read temperature and humidity
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Read MQ sensors
  int sensorValueMQ4 = analogRead(A0);
  int sensorValueMQ7 = analogRead(A1);
  int sensorValueMQ135 = analogRead(A2);

  // Read water pressure
  X = pulseIn(input, HIGH);
  Y = pulseIn(input, LOW);
  TIME = X + Y;
  FREQUENCY = 1000000 / TIME;

  // Read earthquake sensor
  int value1 = analogRead(x);
  int xValue = xsample - value1;

  // Print values for debugging
  Serial.print("Humidity: "); Serial.print(hum); Serial.print("% ");
  Serial.print("Temperature: "); Serial.print(temp); Serial.println("C");
  Serial.print("MQ4: "); Serial.println(sensorValueMQ4);
  Serial.print("MQ7: "); Serial.println(sensorValueMQ7);
  Serial.print("MQ135: "); Serial.println(sensorValueMQ135);
  Serial.print("Water Pressure: "); Serial.println(FREQUENCY);
  Serial.print("Earthquake: "); Serial.println(xValue);

  // Check conditions and activate buzzer if needed
  if (sensorValueMQ135 > 200 || sensorValueMQ4 > 450 || sensorValueMQ7 > 600 ||
      hum > 70 || temp > 50 || (FREQUENCY > 350 && FREQUENCY < 1000) ||
      (xValue < -450 || xValue > 450)) {
    analogWrite(BUZZER_PIN, 50); // Turn on buzzer
  } else {
    analogWrite(BUZZER_PIN, 0); // Turn off buzzer
  }

  // Create a JSON object to send
  LoRa.beginPacket();
  LoRa.print("{");
  LoRa.print("\"Humidity\": "); LoRa.print(hum);
  LoRa.print(", \"Temperature\": "); LoRa.print(temp);
  LoRa.print(", \"MQ4\": "); LoRa.print(sensorValueMQ4);
  LoRa.print(", \"MQ7\": "); LoRa.print(sensorValueMQ7);
  LoRa.print(", \"MQ135\": "); LoRa.print(sensorValueMQ135);
  LoRa.print(", \"WaterPressure\": "); LoRa.print(FREQUENCY);
  LoRa.print(", \"Earthquake\": "); LoRa.print(xValue);
  LoRa.print("}");
  LoRa.endPacket();

  delay(1000); // Delay before sending the next packet
}
