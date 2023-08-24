#include <ArduinoJson.h>

#include <DHT.h>  
 
#include <ESP8266WiFi.h>

#include <Adafruit_Sensor.h>

#include "CTBot.h"

#include <Wire.h>

#include <SFE_BMP180.h>

String apiKey = "xxxx";     //API key from ThingSpeak
 
const char *ssid =  "xxxx";     //Wifi name
const char *pass =  "xxxxx";       //Wifi password
const char* server = "api.thingspeak.com";
 
#define DHTPIN 2          //dht11 is connected  pin d4. write 5 for pin d1

SFE_BMP180 bmp;
double T, P;
char status;

String token = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";   // Telegram bot http api

CTBot myBot;

DHT dht(DHTPIN, DHT11);

WiFiClient client;
 
void setup() 
{
       Serial.begin(115200);
       delay(10);
       dht.begin();
       bmp.begin();
       Wire.begin();
 
       Serial.println("Connecting to ");
       Serial.println(ssid);
 
 
       WiFi.begin(ssid, pass);
 
      while (WiFi.status() != WL_CONNECTED) 
     {
            delay(500);
            Serial.print(".");
     }
 
      Serial.println("");
      Serial.println("...WiFi connected...");

      Serial.println("\n...Starting TelegramBot...");   //esp8266 access point 
      myBot.wifiConnect(ssid, pass);

      myBot.setTelegramToken(token);        //token

      if (myBot.testConnection()){                //connection status
        Serial.println("\n...Test Connection OK");
      }
      else {
        Serial.println("\n...Test Connection NOT OK");
      }

      pinMode(D8, OUTPUT);        //set pin for LED
      digitalWrite(D8, LOW);

} 
void loop() 
{
  
      status =  bmp.startTemperature();       //temperature for pressure
      if (status != 0) {
      delay(status);
      status = bmp.getTemperature(T);

      status = bmp.startPressure(3);      // 0 to 3. if 3 get pressure.
        if (status != 0) {
          delay(status);
          status = bmp.getPressure(P, T);
          if (status != 0) {

          }
        }
      }
      
      float h = dht.readHumidity();      //sensor readings
      float t = dht.readTemperature();
      
      
      
              if (isnan(h) || isnan(t)) 
                 {
                     Serial.println("The DHT sensor is failed!");     
                      return;
                 }
 
                         if (client.connect(server,80))   // for api.thingspeak.com
                      {  
                            
                             String postStr = apiKey;
                             postStr +="&field1=";
                             postStr += String(t);
                             postStr +="&field2=";
                             postStr += String(h);
                             postStr +="&field3=";
                             postStr += String(P, 2);
                             postStr += "\r\n\r\n\r\n";
 
                             client.print("POST /update HTTP/1.1\n");
                             client.print("Host: api.thingspeak.com\n");
                             client.print("Connection: close\n");
                             client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                             client.print("Content-Length: ");
                             client.print(postStr.length());
                             client.print("\n\n");
                             client.print(postStr);
 
                             Serial.print("Temperature: ");
                             Serial.print(t);
                             Serial.print(" degrees Celcius, Humidity: ");
                             Serial.print(h);
                             Serial.print("%, Pressure: ");
                             Serial.print(P,2);
                             Serial.println(" mb");
                             Serial.println(" Send to Thingspeak.");


                             if(t>=23){
                              digitalWrite(D8, HIGH);   // turn the LED on
                              delay(1000);                       
                              digitalWrite(D8, LOW);    // turn the LED off
                              delay(1000);                        
                              Serial.println("Too hot!");
                             }

                             
                             TBMessage msg; //store for telegram data

  
                             if (myBot.getNewMessage(msg)) {        // if comes to message
                              Serial.println("\nReceived:");
                              Serial.println(msg.text);

                             if ( msg.text.equalsIgnoreCase("/start")){
                                String reply = (String)"Hello user! :)" + (String)"\n" + (String)"I'm a Telegram Bot for your IoT Project." + (String)"\n" + (String)"If you want to know the environmental conditions, please send the /read_sensors to me as a message." ;
                                myBot.sendMessage(msg.sender.id, reply);
                               }
                             if(msg.text.equalsIgnoreCase("/read_sensors")){
                                  String reply = (String)"Temperature: " + (String)t + (String)"°C" + (String)"\n" + (String)"Humidity: " + (String)h + (String)"% " + (String)"\n" + (String)"Pressure: " + (String)P + (String)" Pa" ;
                                  myBot.sendMessage(msg.sender.id, reply);
                                }
                             }
                        }
          client.stop();
 
          Serial.println("Waiting...");
  
  //thingspeak 15 sec needs for updating
  delay(1000);
}
