//=====================================gps=====================================
#include <TinyGPS++.h> //https://github.com/mikalhart/TinyGPSPlus
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"



TinyGPSPlus gps;
HardwareSerial neogps(2);
#define RXD2 16 //to tx neo
#define TXD2 17 //to rx neo
#define WIFI_SSID "Redmi Note 10 Pro Max" //wifi name
#define WIFI_PASS "12345678" //your password
#define API_KEY "AIzaSyABS1wx4BOIbUMNGcH9xD4R4oQzIwdTiNs" //your api key of firebase database
#define DATABASE_URL "https://esp-32-firebase-3260b-default-rtdb.asia-southeast1.firebasedatabase.app/" //your url of firebase 


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;


//=====================================Millis=====================================
unsigned long previousMillisTakeData = 0;        // will store last time LED was updated
const long intervalTakeData = 1000;           // intervalTakeData at which to blink (milliseconds)

void setup() {
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);


  WiFi.begin(WIFI_SSID,WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }

  
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }

  
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
}

void loop() {
  unsigned long currentMillis = millis();
  bool newData = false;
  float longi;
  float lati;  
  if (currentMillis - previousMillisTakeData >= intervalTakeData) {
    // save the last time you blinked the LED
    previousMillisTakeData = currentMillis;
    
    while(neogps.available())
    {
      if (gps.encode(neogps.read())) // Parsing semua data
      newData = true;
    }
  }
  if (newData)
  {
    Serial.print(F("Location: ")); 
    if (gps.location.isValid())
    {
      lati =   gps.location.lat();
      longi = gps.location.lng();
Firebase.RTDB.setFloat(&fbdo, "Location/Longitute", longi);
Firebase.RTDB.setFloat(&fbdo, "Location/Latitute", lati);
          
      Serial.print(lati, 6);
      Serial.print(F(","));
      Serial.print(, 6);
    }
    else
    {
      Serial.print(F("INVALID"));
    }
    Serial.println();
}

void storeLocation(double latitude, double longitude) {
  // Create a JSON object
  FirebaseJson json;

  // Add latitude and longitude to the JSON object
  json.set("latitude", latitude, 6);
  json.set("longitude", longitude, 6);

  // Push the data to the "location" node in Firebase
  if (Firebase.updateNode(firebaseData, "/location", json)) {
    Serial.println("Data stored successfully");
  } else {
    Serial.println("Error storing data");
    Serial.println(firebaseData.errorReason());
  }
  delay(1000); // Wait for 1 seconds before sending another update
}