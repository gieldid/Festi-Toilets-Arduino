#include <ESP8266WiFi.h>
#include <Servo.h>  
#include <ArduinoJson.h>
#include <HttpClient.h>
#include <ESP8266HTTPClient.h>

#include "config.h"
#include "settings.h"


int toiletId = TOILET_ID;
int festivalId = FESTIVAL_ID;
char* festivalName = FESTIVAL_NAME; 
int toiletSideId = FESTIVAL_SITE_ID;


String apiAddress = SERVER_URL;
String result;
int servoPin = SERVO_PIN;
int pirPin = PIR_PIN;
String motion ;
Servo servo;  
int status = WL_IDLE_STATUS; 

bool isOccupied = false;

void setup() 
{
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  pinMode(pirPin, INPUT);
  servo.attach(servoPin);
  delay(2000);
  Serial.begin(115200);
  
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WEP network, SSID: ");
    Serial.println(CONFIG_SSID);
    status = WiFi.begin(CONFIG_SSID, CONFIG_PASSWORD);

    // wait 10 seconds for connection:
    delay(10000);
  }
  sendInitial();
  digitalWrite(BUILTIN_LED, HIGH);
}


void sendInitial(){
    String json; 
    const size_t capacity = 2*JSON_ARRAY_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4);
    DynamicJsonDocument doc(capacity);

    doc["id"] = festivalId;
    doc["name"] = festivalName;

    JsonArray toiletSites = doc.createNestedArray("toiletSites");
    
    JsonObject toiletSite = toiletSites.createNestedObject();
    toiletSite["id"] = toiletSideId;
    
    JsonArray toilets = toiletSite.createNestedArray("toilets");
    
    JsonObject toilet = toilets.createNestedObject();
    toilet["id"] = toiletId;
    toilet["isOccupied"] = isOccupied;
    toilet["toiletSite"] = nullptr;
    toiletSite["status"] = 0;
    toiletSite["festival"] = nullptr;
    
    serializeJson(doc, json);
    
    HTTPClient http;
    http.begin(apiAddress + "/api/festivals");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(json);
    String payload = http.getString(); 
    Serial.println(httpCode);
    Serial.println(payload);  
    http.end();
}

void sendUpdate(){
 
  String json;
  
  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc(capacity);
  
  doc["id"] = toiletId;
  doc["isOccupied"] = isOccupied;
  doc["toiletSite"] = nullptr;
  
  serializeJson(doc, json);

  
  HTTPClient http;
  http.begin(apiAddress + "/api/toilets/"+ toiletId);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.PUT(json);
  String payload = http.getString(); 
  Serial.println(httpCode);
  Serial.println(payload);  
  http.end();
}
  

void changeOccupyance(bool isOccupiedNew){
  if(isOccupied != isOccupiedNew){
      isOccupied = isOccupiedNew;
      sendUpdate();
    }
}

  
void loop() 
{
 if(digitalRead(pirPin)== HIGH)  
  {
   motion = digitalRead(pirPin);
   servo.write(10);
   changeOccupyance(true);
  } 
  else 
  {
    servo.write(90);
    changeOccupyance(false); 
  }
}


  
