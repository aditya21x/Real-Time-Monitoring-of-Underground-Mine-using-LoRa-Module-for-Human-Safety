#include <SPI.h>
#include <LoRa.h>
#define BUZZER_PIN 3
#include "DHT.h"
#define DHTPIN A3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int X;
int Y;
float TIME = 0;
float FREQUENCY = 0;
const int input = A4;
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
  dht.begin();
  while (!Serial)
    ;
  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
 

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print("%");
  Serial.print(" Temperature:");
  Serial.print(temp);
  Serial.println("C");
  Serial.println("");
  //*************dht11*********
  int sensorValuemq4 = analogRead(A0);
  Serial.print("mq4--");
  Serial.println(sensorValuemq4);

  //***********mq4*******
  int sensorValuemq7 = analogRead(A1);
  Serial.print("mq7--");
  Serial.println(sensorValuemq7);

  //*************mq7*********
  int sensorValuemq135 = analogRead(A2);
  Serial.print("mq135--");

  Serial.println(sensorValuemq135);
//*********************mq135************************
X = pulseIn(input, HIGH);
  Y = pulseIn(input, LOW);
  TIME = X + Y;
  FREQUENCY = 1000000 / TIME;
  // int sensorvaluewater=analogRead(A4);
  Serial.print("water pressure--");
  Serial.println(FREQUENCY);
  //*************************************water********************
  if (sensorValuemq135 > 200 || sensorValuemq4 > 450 || sensorValuemq7 > 400 || hum > 70 || temp > 35 ||(FREQUENCY>150 && FREQUENCY<1000)) {
    analogWrite(BUZZER_PIN, 50);
  } else {
    analogWrite(BUZZER_PIN, 0);
  }

  LoRa.beginPacket();
  LoRa.print("Humidity: ");
  LoRa.print(hum);
  LoRa.print("%");
  LoRa.print(" Temperature:");
  LoRa.print(temp);
  LoRa.print("C");

   LoRa.print("mq4--");
  LoRa.print(sensorValuemq4);

  LoRa.print("mq7--");
  LoRa.print(sensorValuemq7);

  LoRa.print("mq135--");
  LoRa.print(sensorValuemq135);

  LoRa.print("water pressure");
  LoRa.print(FREQUENCY);


  
  
  LoRa.endPacket();
  delay(1000);
  
}
