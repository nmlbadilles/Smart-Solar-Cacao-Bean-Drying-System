#include "HX711.h"
#include <DS3231.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
//#include "Adafruit_SHT31.h"
#include <SHT2x.h>
#include <SPI.h>
#include <SD.h>


//LCD I2C Module
LiquidCrystal_I2C lcd(0x27, 20, 4);
uint8_t percent[8] = {0x00, 0x19, 0x1B, 0x06, 0x0C, 0x1B, 0x13, 0x00};
uint8_t celcs[8] = {0x00, 0x10, 0x07, 0x04, 0x04, 0x04, 0x07, 0x00};

//SHT31 Module
//Adafruit_SHT31 sht31 = Adafruit_SHT31();
float t;
float h;

//SD
const int chipSelect = 53;
File myFile;

//RTC Module
DS3231  rtc(SDA, SCL);
String myTime;
String myDate;

//LoadCell
#define DOUT  31
#define CLK  30
HX711 scale;
float calibration_factor = 203; 
float weight;
float weight1;
float weight2;
float exitweight;
float wMin;
float wMax;

//RelayModule
const int relayFan = 22;
const int relayHeater = 23;
const int relayLight = 24;
const int relayAlarm = 25;

//Accessories
const int ledRed = 8;
const int ledGreen = 10;
const int button = 7;
int buttonState = 0;
String fanStat = "Fan OFF";
String heaterStat = "Heater OFF";
String stat = "DRYING ONGOING";
const unsigned long SECOND = 1000;
const unsigned long MINUTE = 60*SECOND;
const unsigned long delayTime = 5*SECOND;
float weights[] = {1, 0, 0, 0, 0, 0, 10};

        void setup() {
          // put your setup code here, to run once:
         //LCD I2C Module
         lcd.begin();
         lcd.backlight();
         lcd.clear();
         lcd.createChar(1, percent);
         lcd.createChar(2, celcs);
        
         Serial.begin(9600); 
          
         //RTC and SHT31
         //sht31.begin(0x44);
         Wire.begin();
         rtc.begin();
         
        //SD
         SD.begin();
          while (!Serial) {
          ; // wait for serial port to connect. Needed for native USB port only
          }
          
          Serial.print("Initializing SD card...");
          
          if (!SD.begin(chipSelect)) {
          Serial.println("initialization failed!");
          while (1);
          }
          Serial.println("initialization done."); 
        
          //LoadCell
         scale.begin(DOUT, CLK);
         scale.set_scale(calibration_factor); //Adjust to this calibration factor
         scale.tare(); 
        
        //Relay
         pinMode(relayFan, OUTPUT);
         pinMode(relayHeater, OUTPUT);
         pinMode(relayLight, OUTPUT); 
         pinMode(relayAlarm, OUTPUT);
        
         //Accessories
         pinMode(ledRed, OUTPUT);
         pinMode(ledGreen, OUTPUT); 
         pinMode(button, INPUT);
         }

       void buttonWait(int buttonPin){
        while(1){
          buttonState = digitalRead(buttonPin);
          if (buttonState == HIGH) {
            return;
            }
          }
        }
        
        void pauseCode(){
          while(1){
            exitweight = scale.get_units(),3;
            if(exitweight < 20){
              return;
            }
          }
        }
        
        void loopCode(){
          weights[0] = 1;
          weights[6] = 10;

              for (int i = 0; i <7; i++){
                        //Weight
                       weight = scale.get_units(), 3;
                       if (weight < 0){
                         weight = 0.000;
                       }
                        //Temperature and Humidity

                        t = SHT2x.GetTemperature();
                        h = SHT2x.GetHumidity();
                       //t = sht31.readTemperature();
                       //h = sht31.readHumidity();  
                      
                        //Time and Date
                        myTime = rtc.getTimeStr();
                        myDate = rtc.getDateStr();
              
                          if(t < 33){
                            digitalWrite(relayFan, HIGH);
                            digitalWrite(relayHeater, HIGH);
                            fanStat = "Fan ON";
                            heaterStat = "Heater ON";
                          } else if(t >= 33 && t <= 45){
                            digitalWrite(relayFan, LOW);
                            digitalWrite(relayHeater, LOW);
                            fanStat = "Fan OFF";
                            heaterStat = "Heater OFF";
                          } else if (t > 45){
                            digitalWrite(relayFan, HIGH);
                            digitalWrite(relayHeater, LOW);
                            fanStat = "Fan ON";
                            heaterStat = "Heater OFF";
                          }
                        
                        // Serial
                        Serial.print(myDate);
                        Serial.print(" -- ");
                        Serial.print(myTime);
                        Serial.print(" -- ");
                        Serial.print(t);
                        Serial.print(" C");
                        Serial.print(" -- ");
                        Serial.print(h);
                        Serial.print(" %");
                        Serial.print(" -- ");
                        Serial.print(weight);
                        Serial.print(" g");
                        Serial.print(" -- ");
                        Serial.print(fanStat);
                        Serial.print(" -- ");
                        Serial.println(heaterStat);
                        
                        //SD Card Module
                       myFile = SD.open("Datalog.txt", FILE_WRITE);
                        if (myFile){
                          myFile.print(myDate);
                          myFile.print(",");
                          myFile.print(myTime);
                          myFile.print(",");
                          myFile.print(t);
                          myFile.print(",");
                          myFile.print(h);
                          myFile.print(",");
                          myFile.print(weight);
                          myFile.print(",");
                          myFile.print(fanStat);
                          myFile.print(",");
                          myFile.println(heaterStat);
                          myFile.close();
                        }

                       weights[i] = scale.get_units(), 3;
                       
                       lcd.setCursor(0,0);
                       lcd.print("Date: ");
                       lcd.setCursor(6,0);
                       lcd.print(myDate);

                       lcd.setCursor(0,1);
                       lcd.print("Time: ");
                       lcd.setCursor(6,1);
                       lcd.print(myTime);
/*
                       lcd.setCursor(0,1);
                       lcd.print(weights[i]);
                       */
                       lcd.setCursor(0,2);
                       lcd.print("Temp: ");
                       lcd.setCursor(6,2);
                       lcd.print(t,3);
                       lcd.setCursor(13,2);
                       lcd.write(2);
              
                       lcd.setCursor(0,3);
                       lcd.print(stat);

                       delay(delayTime);
              }
        }

        void loop() {
        //initialization
        digitalWrite(ledRed, LOW);
        pinMode(ledGreen, LOW); 
        digitalWrite(relayFan,LOW); 
        digitalWrite(relayHeater,LOW);
        digitalWrite(relayLight,LOW);
        digitalWrite(relayAlarm,LOW);
        
        stat = "DRYING ONGOING";
        fanStat = "Fan OFF";
        heaterStat = "Heater OFF";

        weights[0] = 1;
        weights[6] = 10;

       lcd.setCursor(0,0);
       lcd.print(" SMART SOLAR CACAO ");
       lcd.setCursor(0,1);
       lcd.print("    BEAN DRYING    ");
       lcd.setCursor(0,2);
       lcd.print("       SYSTEM       ");
       lcd.setCursor(0,3);
       lcd.print(" BADILLES X BONLEON ");

       //Significant weight detection
       weight1 = scale.get_units(), 3;
       
       if (weight1 > 30){
        digitalWrite(ledRed, HIGH);
        delay(3000);
        weight2 = scale.get_units(), 3;
        wMin = weight2 - 5;
        wMax = weight2 + 5;
        
        if(weight1 >= wMin && weight1 <= wMax){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("  PRESS BUTTON TO  ");
          lcd.setCursor(0,1);
          lcd.print("    BEGIN DRYING    ");

          buttonWait(button);
            lcd.clear();
            digitalWrite(ledRed, LOW);
            digitalWrite(ledGreen, HIGH);
            
            while(weights[0] < (weights[6] - 3) || weights[0] > (weights[6] + 3)){
                  loopCode();    
              }
            
            digitalWrite(relayFan, LOW);
            digitalWrite(relayHeater, LOW);
            digitalWrite(relayLight, HIGH);
            digitalWrite(relayAlarm, HIGH);
            digitalWrite(ledGreen, HIGH);

            //Weight
             weight = scale.get_units(), 3;
             if (weight < 0){
               weight = 0.000;
             }
            
            //Temperature and Humidity
             t = SHT2x.GetTemperature();
             h = SHT2x.GetHumidity();
             //t = sht31.readTemperature();
             //h = sht31.readHumidity();  
            
            //Time and Date
            myTime = rtc.getTimeStr();
            myDate = rtc.getDateStr();

             //SD Card Module
             myFile = SD.open("Datalog.txt", FILE_WRITE);
              if (myFile){
                myFile.print(myDate);
                myFile.print(",");
                myFile.print(myTime);
                myFile.print(",");
                myFile.print(t);
                myFile.print(",");
                myFile.print(h);
                myFile.print(",");
                myFile.print(weight);
                myFile.print(fanStat);
                myFile.print(",");
                myFile.println("DRYING COMPLETE");
                myFile.close();
              }
               lcd.clear();
               lcd.setCursor(0,0);
               lcd.print(myDate);
               lcd.setCursor(12,0);
               lcd.print(myTime);
                              
               lcd.setCursor(0,1);
               lcd.print("  DRYING COMPLETE");

               lcd.setCursor(0,2);
               lcd.print(" PRESS BUTTON AFTER ");
               
               lcd.setCursor(0,3);
               lcd.print("  COLLECTING BEANS  ");

               buttonWait(button);
               lcd.clear();
          }
         }
         //pauseCode();
        }
