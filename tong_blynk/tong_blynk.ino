#define BLYNK_TEMPLATE_ID "TMPLI6ED0BTb"
#define BLYNK_DEVICE_NAME "Test Blynk"
#define BLYNK_AUTH_TOKEN "rDDQy9mFrmA0uuvOB_EZdHG2NqzP-G-B"
// Include the necessary libraries
#include <Wire.h>
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Define the pin numbers
#define ECHO_PIN 12         // Input pin for the distance sensor
#define TRIGGER_PIN 13      // Output pin for the distance sensor
#define XA 4                // Relay pin for val motor control
#define BOM 5               // Relay pin for pump motor control
#define oneWireBus 18       // OneWire bus pin

// Create instances of necessary libraries
OneWire oneWire(oneWireBus);                        // OneWire instance
DallasTemperature sensors(&oneWire);                // DallasTemperature instance

// Variables
unsigned long duration;                             // Time duration for distance measurement
int distance;                                       // Distance value
int sw_mode, pump, val;
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Quoc Viet";                          // Wi-Fi network SSID
char pass[] = "quocviet2002";                       // Wi-Fi network password

BLYNK_WRITE(V5) // Button Widget writes to Virtual Pin V5 
  {
      sw_mode = param.asInt();  
  }

BLYNK_WRITE(V3) // Button Widget writes to Virtual Pin V5 
{
      pump = param.asInt();  
}
BLYNK_WRITE(V4) // Button Widget writes to Virtual Pin V5 
{
      val = param.asInt();  
}

void setup() {
  // Set trạng thái các chân ra hoặc vào
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(XA, OUTPUT);
  pinMode(BOM, OUTPUT);

  // Initialize Serial communication
    Serial.begin(115200);                              
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  // Kết nối Wifi
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Blynk.begin(auth, ssid, pass);
  // Start the DS18B20 sensor
    sensors.begin();                                 
}

// Đo mực nước
int getWaterLevel() {
  long duration;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = int(duration / 2 / 29.412);
  return distance;
}

// In khoảng cách lên serial monitor
void printDistance() {
  int waterLevel = getWaterLevel();
  Serial.print("Distance: ");
  Serial.print(waterLevel);
  Serial.println("cm");

  Blynk.virtualWrite(V2, waterLevel);  // Send water level value to virtual pin V1 in Blynk app
}

// Chương trình đo nhiệt độ bằng ds18b20
void measureTemperature() {
  sensors.requestTemperatures();                    // Gửi yêu cầu đo nhiệt độ từ cảm biến
  float temperatureC = sensors.getTempCByIndex(0);  // Đọc nhiệt độ đo được theo độ Celsius từ cảm biến
  float temperatureF = sensors.getTempFByIndex(0);  // Đọc nhiệt độ đo được theo độ Fahrenheit từ cảm biến
  Serial.print("Temperature: ");                    // In ra dòng chữ "Temperature: " trên cổng Serial
  Serial.print(temperatureC);                       // In ra giá trị nhiệt độ Celsius trên cổng Serial         
  Serial.print("°C  ");                             // In ra ký tự độ Celsius trên cổng Serial
  Serial.print(temperatureF);                       // In ra giá trị nhiệt độ Fahrenheit trên cổng Serial
  Serial.println("°F");                             // In ra ký tự độ Fahrenheit và xuống dòng trên cổng Serial
  Blynk.virtualWrite(V0, temperatureC);             // Send temperature value to virtual pin V2 in Blynk app
  Blynk.virtualWrite(V1, temperatureF);             // Send temperature value to virtual pin V2 in Blynk app
}

void automode() {
  int waterLevel = getWaterLevel();                 // Lấy giá trị mức nước từ cảm biến
  if (waterLevel > 10) {                            // Nếu mức nước lớn hơn 10
    digitalWrite(XA, HIGH);                         // Bật motor theo chiều một chiều
    digitalWrite(BOM, LOW);                         // Tắt motor theo chiều ngược lại
  }
  else if (waterLevel >= 6 && waterLevel <= 10) {   // Nếu mức nước nằm trong khoảng từ 6 đến 10
    digitalWrite(XA, LOW);                          // Tắt motor theo chiều một chiều
    digitalWrite(BOM, LOW);                         // Tắt motor theo chiều ngược lại
  }
  else if (waterLevel < 6) {                        // Nếu mức nước nhỏ hơn 6
    digitalWrite(BOM, HIGH);                        // Bật motor theo chiều ngược lại
    digitalWrite(XA, LOW);                          // Tắt motor theo chiều một chiều
  }
}


void manualmode() {
  if(pump == 1)                                    // nếu pump(switch) mức cao
 
  {
     digitalWrite(BOM, HIGH);                      // kích relay máy bơm
  }
  else                                             // nếu pump mức thấp
  {
    digitalWrite(BOM, LOW);                        // tắt máy bơm     
  }
   if (val == 1)                                   // nếu xả (switch) múc cao
  {  
     digitalWrite(XA, HIGH);                       // Bật xả nước
  }
  else                                             // nếu xả mức thấp
  {
     digitalWrite(XA, LOW);                        // tắt xả
  }
}
void loop() {
  delay(200);            // chờ 1s
  if (sw_mode ==1)       // nếu chuyển qua chế dộ tự động
  { 
     automode();         // qua chế độ auto
  }
  else
  {                      // nếu ngược lại
     manualmode();       // qua chế độ tự chỉnh
  }
  measureTemperature();  // chương trình đo nhiệt độ
  printDistance();       // chương trình in khoảng cách lên serial
  Blynk.run();  // Run Blynk // chạy blynk
  //delay(200);
}
