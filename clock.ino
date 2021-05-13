
#include "SPI.h"
#include "Adafruit_SSD1306.h"
#include <Wire.h>
#include <RTClib.h>




// If using software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

#define CLOCK_INTERRUPT_PIN 2



Adafruit_SSD1306 OLED(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
RTC_DS3231 rtc; 
DateTime now;
DateTime alarmtime;

String time_text = "H : m : s";
boolean blink_toggle = false;
boolean LED_blink_toggle = false;
boolean isLEDBlinking = false;
int setting_mode = 0;
int BlinkingCount = 0;

//-------------------------------------------------------------------
void setup() {
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  rtc.disable32K();
  
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  rtc.writeSqwPinMode(DS3231_OFF);
  rtc.disableAlarm(2);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), SetAlarmBlink, FALLING);
  
  OLED.begin(SSD1306_SWITCHCAPVCC,0xAA); // initialize with the  addr 0x3C 128x64)
  
  Serial.begin(9600);
  alarmtime = DateTime(2020, 1, 1, 23,32, 0);
  if(rtc.setAlarm1(alarmtime, DS3231_A1_Hour)){
    Serial.println("Alarm was set");
  }

  now = rtc.now();
  time_text = String(now.hour()) +  " : " + String(now.minute()) + " : " + String(now.second());
}

void loop(){

  if(setting_mode == 0)
  {
    now = rtc.now();
  } 
  show();
  
  if(isLEDBlinking)
  {
    onAlarm();
    isLEDBlinking = false;
    OLED.begin(SSD1306_SWITCHCAPVCC,0xAA); // initialize with the  addr 0x3C 128x64)
  }
    
  if(rtc.alarmFired(1)) {
        rtc.clearAlarm(1);
        Serial.println("Alarm cleared");
  }
  
  if(Serial.available())
  {
    char a = Serial.read();
    if (a == '1') // left button
    {
      setting_mode = (setting_mode + 1) % 6;
      if(setting_mode == 0){ // turn back to clock mode
        rtc.clearAlarm(1);
        if(rtc.setAlarm1(alarmtime, DS3231_A1_Hour)){
          Serial.println("Alarm was set");
        }
        rtc.adjust(now);
      }
      
    }
    if (a == '2') // right button
    {
      
      if(setting_mode == 1){ // alarm hour setting
          int new_hour = (alarmtime.hour() + 1)%24;
          alarmtime = DateTime(2020, 1, 1, new_hour, alarmtime.minute(), 0);
      }
      else if (setting_mode == 2){ // alarm minute setting
          int new_minute = (alarmtime.minute() + 1)%60;
          alarmtime = DateTime(2020, 1, 1, alarmtime.hour(), new_minute, 0);
      }

      else if (setting_mode == 3){ // clock hour setting
          int new_hour = (now.hour() + 1)%24;
          now = DateTime(2020, 1, 1, new_hour, now.minute(), now.minute());
      }
      else if (setting_mode == 4){ // clock minute setting
          int new_minute = (now.minute() + 1)%60;
          now = DateTime(2020, 1, 1, now.hour(), new_minute, now.minute());
      }
      else if (setting_mode == 5){ // clock second setting
          int new_second = (now.second() + 1)%60;
          now = DateTime(2020, 1, 1, now.hour(), now.minute(), new_second);
      }
      
    }
    if(a == '3')
    {
      onAlarm();
      OLED.begin(SSD1306_SWITCHCAPVCC,0x3C); // initialize with the  addr 0x3C 128x64)
    }
  }

  delay(300); 
  
}

void show(){
  OLED.clearDisplay(); 
  OLED.setTextColor(WHITE);   //Text is white ,background is black
  OLED.setTextSize(1);
  
  OLED.setCursor(0,0);
  if(setting_mode == 0)
  {
     time_text = String(now.hour()) +  " : " + String(now.minute()) + " : " + String(now.second());
     
  }
  else if(setting_mode == 3)
  {
    if(blink_toggle){
      time_text = "__ : " + String(now.minute()) + " : " + String(now.second());
      blink_toggle = !blink_toggle;
    }
    else{
      time_text = String(now.hour()) +  " : " + String(now.minute()) + " : " + String(now.second());
      blink_toggle = !blink_toggle;
    }
  }
  else if(setting_mode == 4)
  {
     if(blink_toggle){
      time_text = String(now.hour()) +  " : " + "__" + " : " + String(now.second());
      blink_toggle = !blink_toggle;
     }
     else{
      time_text = String(now.hour()) +  " : " + String(now.minute()) + " : " + String(now.second());
      blink_toggle = !blink_toggle;
    }
  }
  else if(setting_mode == 5)
  {
    if(blink_toggle){
      time_text = String(now.hour()) +  " : " + String(now.minute()) + " : " + "__";
      blink_toggle = !blink_toggle;
    }
    else{
      time_text = String(now.hour()) +  " : " + String(now.minute()) + " : " + String(now.second());
      blink_toggle = !blink_toggle;
    }
  }
  OLED.println(time_text);

  
  
  OLED.drawLine(0, 15, 127,15, WHITE);


  
  OLED.setCursor(0,25);
  OLED.println("alarm");
  
  OLED.setCursor(50,25);
  
  if(setting_mode == 0 || setting_mode == 3 || setting_mode == 4 || setting_mode == 5){  // blink hour / minute
     OLED.println(String(alarmtime.hour()) + " : " + String(alarmtime.minute()));
  }
  else if(setting_mode == 1){
    if(blink_toggle){
      
      OLED.println(String(alarmtime.hour()) + " : " + String(alarmtime.minute()));
      blink_toggle = !blink_toggle;
    }
    else{
      OLED.println("__ : " + String(alarmtime.minute()));
      blink_toggle = !blink_toggle;
    }
  }
  else if(setting_mode == 2){
    if(blink_toggle){
      
      OLED.println(String(alarmtime.hour()) + " : " + String(alarmtime.minute()));
      blink_toggle = !blink_toggle;
    }
    else{
      OLED.println(String(alarmtime.hour()) + " : __");
      blink_toggle = !blink_toggle;
    }
  }
  OLED.display(); 
  
  
}

void SetAlarmBlink(){
    isLEDBlinking = true;
    
}

void onAlarm(){
  int alaert_count = 5;
  for(int i=0 ; i<alaert_count ; i++){
    
    OLED.setTextColor(WHITE);   //Text is white ,background is black
    OLED.setTextSize(1);
  
    OLED.setCursor(0,15);
    OLED.println("!!!  Wake up  !!!");
    OLED.display(); 
    digitalWrite(LED_BUILTIN, HIGH) ; 
    delay(300);
    
    OLED.clearDisplay(); 
    OLED.display(); 
    digitalWrite(LED_BUILTIN, LOW) ; 
    delay(200);
    
  }
}
  
  
  
