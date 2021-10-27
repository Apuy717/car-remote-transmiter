#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include "I2Cdev.h"  //For communicate with MPU6050
#include "MPU6050.h" //The main library of the MPU6050

/* Define WiFi Credentials */
const char *ssid = "ESP-APUYSTEREO";
const char *password = "12345678QWERTY";

//Define the object to access and cotrol the Gyro and Accelerometer (We don't use the Gyro data)
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

//Define packet for the direction (X axis and Y axis)
int data[2];

void indicatorDisconected()
{
  digitalWrite(D4, HIGH);
  delay(500);
  digitalWrite(D4, LOW);
  delay(500);
}

void indicatorConnected()
{
  digitalWrite(D4, HIGH);
  digitalWrite(D4, LOW);
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  pinMode(D4, OUTPUT); //initialise the D4 indicator
  mpu.initialize();    //Initialise the MPU object

  // set the ESP8266 to be a WiFi-client
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    indicatorDisconected();
    Serial.println("...");
    delay(1000);
  }
  indicatorConnected();
  Serial.print("wifi connected ip = ");
  Serial.println(WiFi.localIP().toString());
  delay(5000);
}

void loop()
{
  //indicator connected
  indicatorConnected();
  // We now create a URI for the request. Something like /data/?sensor_reading=123
  String url = "/?direction=S";

  //With this function, the acceleration and gyro values of the axes are taken.
  //If you want to control the car axis differently, you can change the axis name in the map command.
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  //In two-way control, the X axis (data [0]) of the MPU6050 allows the robot to move forward and backward.
  //Y axis (data [0]) allows the robot to right and left turn.
  data[0] = map(ax, -17000, 17000, 300, 400); //Send X axis data
  data[1] = map(ay, -17000, 17000, 100, 200); //Send Y axis data
  Serial.print("data x = ");
  Serial.println(data[1]);
  Serial.print("data y = ");
  Serial.println(data[0]);

  if (data[0] > 380)
  {
    //forward car
    url = "/?direction=F";
    Serial.println("---------------------------------------------------");
    Serial.println("FORWARD");
    Serial.println("---------------------------------------------------");
  }

  if (data[0] < 310)
  {
    //backward car
    url = "/?direction=B";
    Serial.println("---------------------------------------------------");
    Serial.println("BACKWARD");
    Serial.println("---------------------------------------------------");
  }

  if (data[1] > 180)
  {
    //left car
    url = "/?direction=L";
    Serial.println("---------------------------------------------------");
    Serial.println("LEFT");
    Serial.println("---------------------------------------------------");
  }

  if (data[1] < 110)
  {
    //right car
    url = "/?direction=R";
    Serial.println("---------------------------------------------------");
    Serial.println("RIGHT");
    Serial.println("---------------------------------------------------");
  }

  if (data[0] > 330 && data[0] < 360 && data[1] > 130 && data[1] < 160)
  {
    //stop car
    url = "/?direction=S";
    Serial.println("---------------------------------------------------");
    Serial.println("STOP");
    Serial.println("---------------------------------------------------");
  }

  WiFiClient client;
  const char *host = "192.168.4.1"; //default IP address
  const int httpPort = 80;

  if (!client.connect(host, httpPort))
  {
    Serial.println("connection failed");
    indicatorDisconected();
    return;
  }

  // This will send the request to the server
  Serial.print("url query = ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
}