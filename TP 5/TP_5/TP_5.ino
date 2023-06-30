/*
  Geronimo Martos y Denis Bae
*/
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "time.h"
#include "DHT.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"

// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "ORT-IoT"
#define WIFI_PASSWORD "OrtIOTnew22$2"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCpMIjQvVPQzP0ktGFFTpTjFTWUUQl41aU"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "geroydendo@gmail.com"
#define USER_PASSWORD "esepe32"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://st-tp5-esp32-firebase-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;

// Database child nodes
String tempPath = "/temperature";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;
long timerDelay = 30000; // 30 segundos
const char* ntpServer = "pool.ntp.org";

#define OLED_RESET 4      // Pin de reset para el display OLED
#define SCREEN_WIDTH 128  // Ancho del display OLED
#define SCREEN_HEIGHT 32  // Alto del display OLED5
#define DHTPIN 23   // Pin del sensor DHT11
#define SW1_PIN 34  // Pin del botón SW1
#define SW2_PIN 35  // Pin del botón SW2
#define DHTPIN 23
#define DHTTYPE DHT11

#define PANTALLA_PRINCIPAL 1    //estados de la maquina
#define ESPERA_CAMBIAR 2
#define PANTALLA_CAMBIAR 3
#define ESPERA_PRINCIPAL 4

DHT dht(DHTPIN, DHTTYPE);

float temp;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int current_screen = 1;
float temp_umbral = 28;
bool flagSubir, flagBajar;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;



// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Initialize BME280 sensor
  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop() {

  switch (current_screen) {

    case PANTALLA_PRINCIPAL:

      if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {

        sendDataPrevMillis = millis();
        temp = dht.readTemperature();
        timestamp = getTime();
        Serial.print ("time: ");
        Serial.println (timestamp);
        parentPath = databasePath + "/" + String(timestamp);
        json.set(tempPath.c_str(), String(dht.readTemperature()));
        Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
        temp = dht.readTemperature();
        timestamp = getTime();
        Serial.print ("time: ");
        Serial.println (timestamp);
        parentPath = databasePath + "/" + String(timestamp);
        json.set(tempPath.c_str(), String(dht.readTemperature()));
        Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(temp);
        display.println(temp_umbral);
        display.display();
      }

        temp = dht.readTemperature();
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(temp);
        display.println(temp_umbral);
        display.display();
        
      if (digitalRead(SW1_PIN) == LOW && digitalRead(SW2_PIN) == LOW) {
        Serial.println("botones apretados");
        current_screen = ESPERA_CAMBIAR;
      }

      break;

    case ESPERA_CAMBIAR:

      if (digitalRead(SW1_PIN) == HIGH && digitalRead(SW2_PIN) == HIGH)
      {
        current_screen = PANTALLA_CAMBIAR;
      }


      break;

    case PANTALLA_CAMBIAR:

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(timerDelay);
      display.println("SW1 para -30s");
      display.println("SW2 para +30s");
      display.display();

      if (digitalRead(SW2_PIN) == LOW)
      {
        flagBajar = HIGH;
      }
      if (digitalRead(SW2_PIN) == HIGH && flagBajar == HIGH)
      {
        flagBajar = LOW;
        timerDelay = timerDelay - 30000;
        if (timerDelay < 0) {
          timerDelay = 0;
        }
      }

      if (digitalRead(SW1_PIN) == LOW)
      {
        flagSubir = HIGH;
      }
      if (digitalRead(SW1_PIN) == HIGH && flagSubir == HIGH)
      {
        flagSubir = LOW;
        timerDelay = timerDelay + 30000;
      }

      if (digitalRead(SW1_PIN) == LOW && digitalRead(SW2_PIN) == LOW)
      {
        Serial.println("botones apretados");
        current_screen = ESPERA_PRINCIPAL;
      }


      break;


case ESPERA_PRINCIPAL:

      if (digitalRead(SW1_PIN) == HIGH && digitalRead(SW2_PIN) == HIGH)
      {
        Serial.println("pasar a pantalla principal");
        current_screen = PANTALLA_PRINCIPAL;
      }


      break;


  }
}
