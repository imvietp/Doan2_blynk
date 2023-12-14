#define BLYNK_TEMPLATE_ID "TMPLI6ED0BTb" //mã định danh cho mẫu (template) dự án trong Blynk
#define BLYNK_DEVICE_NAME "Test Blynk"   //tên của thiết bị được hiển thị trong ứng dụng Blynk khi thiết bị kết nối thành công.
#define BLYNK_AUTH_TOKEN "rDDQy9mFrmA0uuvOB_EZdHG2NqzP-G-B" //mã thông báo (token) xác thực của thiết bị khi kết nối với Blynk. Mã thông báo này được sử dụng để xác thực và cho phép truy cập vào các tài nguyên và chức năng của dự án Blynk.
// Include the necessary libraries
#include <Wire.h>                  //thư viện giao tiếp i2c
#include <LiquidCrystal_I2C.h>     //thư viện lcd i2c
#include <OneWire.h>               //thư viện dây cb ds18b20
#include <DallasTemperature.h>     //thư viện cb ds18b20
#include <WiFi.h>                  //thư viện wifi
#include <BlynkSimpleEsp32.h>      //thư viện blynk
#include <ESP32Servo.h>            //thư viện động cơ servo
#include <RTClib.h>                //thư viện rtos ds1307
// Define the pin numbers
#define ECHO_PIN 12         // Input pin for the distance sensor
#define TRIGGER_PIN 13      // Output pin for the distance sensor
#define XA 4                // Relay pin for val motor control
#define BOM 5               // Relay pin for pump motor control
#define oneWireBus 18       // OneWire bus pin
#define sensorPin  34
// Create instances of necessary libraries
OneWire oneWire(oneWireBus);                        // OneWire instance
DallasTemperature sensors(&oneWire);                // DallasTemperature instance
Servo servo;
// Variables
int servo_onoff = 0;                                // khởi tạo biến servo
unsigned long duration;                             // Time duration for distance measurement
int distance;    
int sensorValue;                                   // Distance value
int sw_mode, pump, val;                             // khởi tạo biến liên quan tới máy bơm
char auth[] = BLYNK_AUTH_TOKEN;                       // token (chứng thực) của blynk 
char ssid[] = "mitom2trung";                         // Wi-Fi network SSID
char pass[] = "66666666";                       // Wi-Fi network password

WidgetLED led1(V9);

RTC_DS1307 rtc;                                    // khởi tạo một đối tượng của lớp RTC_DS1307, cho phép tương tác với mô-đun RTC DS1307. Đối tượng này được đặt tên là rtc.
LiquidCrystal_I2C lcd(0x27, 16,2);                 // khởi tạo biến lcd trong liquidcrystal
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
BLYNK_WRITE(V6) // Button Widget writes to Virtual Pin V5 (nút trong blynk)
{
      servo_onoff = param.asInt();  
}

BLYNK_WRITE(V5) // Button Widget writes to Virtual Pin V5 (nút trong blynk)
  {
      sw_mode = param.asInt();  
  }

BLYNK_WRITE(V3) // Button Widget writes to Virtual Pin V5 (nút trong blynk)
{
      pump = param.asInt();  
}
BLYNK_WRITE(V4) // Button Widget writes to Virtual Pin V5 (nút trong blynk)
{
      val = param.asInt();  
}

void setup() {
  // Set trạng thái các chân ra hoặc vào
    servo.attach(19);  // động cơ servo ở chân 19 của esp
    pinMode(ECHO_PIN, INPUT); // hcsr04
    pinMode(TRIGGER_PIN, OUTPUT); // hcsr04
    pinMode(XA, OUTPUT);  // relay xả
    pinMode(BOM, OUTPUT); // relay bơm

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

    
  // khởi tạo lcd 
    lcd.init();                         // initialize the lcd 
    lcd.init();
    lcd.backlight();                    //To Power ON the back light
    if (! rtc.begin())                  // nếu rtos không kết nối
     {
       lcd.print("Couldn't find RTC");  // màn hình lcd hiện ko tìm thấy rtos
       while (1);                       // để vòng lặp vô tân cho tới khi có rtos kết nối
     }
    if (! rtc.isrunning())              // nếu rtos không chạy, hoạt động
      {
        lcd.print("RTC is NOT running!");  // màn hình lcd hiện rtos không chạy
      }
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));   //rtos tự cập nhật thời gian từ máy tính
    // Start the DS18B20 sensor
    sensors.begin();       // sensor ds18b20 bắt đầu hoạt động                            
}
// hiển thị ngày và giờ lên LCD
void LCD()
{
    DateTime now = rtc.now();
    lcd.setCursor(0, 1);
    lcd.print("TIME");
    lcd.print(" ");
    lcd.print(now.hour());
    lcd.print(':');
    lcd.print(now.minute());
    lcd.print(':');
    lcd.print(now.second());
    lcd.print("  ");

    lcd.setCursor(0, 0);
    lcd.print("DATE");
    lcd.print(" ");
    //lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    //lcd.print(" ");
    lcd.print(now.day());
    lcd.print('/');
    lcd.print(now.month());
    lcd.print('/');
    lcd.print(now.year());
    lcd.print("  ");    
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

// mode tự động của bơm nước
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

// mode tự chỉnh của bơm nước
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

// nút cho cá ăn
void fishfeeder()
{
  if(servo_onoff == 1)
  {
    servo.write(0);  
  }
  else if (servo_onoff == 0)
  {
     servo.write(180);
  }
}

void doduc()
{
  sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);
  int turbidity = map(sensorValue, 0, 2600, 100, 0);
  delay(100);
  Serial.print("turbidity:");
  Serial.print("   ");
  Serial.print(turbidity);
  delay(100);
  if (turbidity < 20) {

    Serial.print(" its CLEAR ");
    led1.off();
  }
  if ((turbidity > 20) && (turbidity < 50)) {

    Serial.print(" its CLOUDY ");
    led1.off();
  }
  if (turbidity > 50) {

    Serial.print(" its DIRTY ");
    led1.on();
  }
   Blynk.virtualWrite(V7, turbidity); 
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
  fishfeeder();          // chương trình cho cá ăn
  measureTemperature();  // chương trình đo nhiệt độ
  printDistance();       // chương trình in khoảng cách lên serial
  doduc();
  LCD();                 // hiển thị ngày và giờ lên lcd
  Blynk.run();  // Run Blynk // chạy blynk

}
