
#include <HardwareSerial.h>
#include <TinyGPS++.h>



#define WIFI_SSID "WAVLINK_46FA"
#define API_KEY "AIzaSyABS1wx4BOIbUMNGcH9xD4R4oQzIwdTiNs"
#define DATABASE_URL "https://esp-32-firebase-3260b-default-rtdb.asia-southeast1.firebasedatabase.app/" 
#define RX_PIN 17  // Connect NEO-6M TX to Arduino pin 2
#define TX_PIN 16  // Connect NEO-6M RX to Arduino pin 3


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;




HardwareSerial neoSerial(1);  // Using Serial1 for NEO-6M communication
TinyGPSPlus gps;


unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void setup(){
  Serial.begin(115200);
  neoSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  WiFi.begin(WIFI_SSID);
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

void loop(){

    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;

    while (neoSerial.available() > 0) {
    if (gps.encode(neoSerial.read())) {
      printGPSInfo();
    }
  }

}

void printGPSInfo() {
  Serial.println("GPS Data:");
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);
  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);
  Serial.print("Speed (km/h): ");
  Serial.println(gps.speed.kmph());
  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());
  Serial.print("Altitude (meters): ");
  Serial.println(gps.altitude.meters(), 0);
  Serial.println("----------------------");
}




