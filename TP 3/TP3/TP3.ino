/* Geronimo Martos y Denis Bae**/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// credenciales de wifi
const char* ssid = "ORT-IoT";
const char* password = "OrtIOTnew22$2";

// Initialize Telegram BOT
#define BOTtoken "6235430638:AAGYaEncOpMHo7J3kIw5olG0G2n1nZYe9t4"  // cambiar por token

#define  CHAT_ID "1838958839" ///

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000; /// intervalo
unsigned long lastTimeBotRan; /// ultimo tiempo

const int ledPin = 2; /// pin led
bool ledState = LOW;
//-----


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//TEMPERATURA



#define DHTPIN 23
#define DHTTYPE DHT11
#define TEMP 27

#define LED 25
#define SW1 34
#define SW2 35

#define PANTALLA_UNO 0
#define ESPERA_1 1
#define ESPERA_2 2
#define ESPERA_3 3
#define ESPERA_4 4
#define ESPERA_5 5
#define PANTALLA_DOS 6
#define RESTA 7
#define SUMA 8
#define ESPERA_6 9

#define INTERVALO 5000
#define INTERVALO2 30000

DHT dht(DHTPIN, DHTTYPE);

float temperaturaActual;
float tempVieja;

int funcionamiento;

int lecturaSW1;
int lecturaSW2;

int valorUmbral;

int hora;
int minuto;
int seg;
int minuto2;
int seg2;
int minutos = 60;
int segundos = 30;

unsigned long currentMillis;
unsigned long currentMillis2;

boolean flagAlerta;

void setup() {

  //telegram
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  ///bloquea el programa si no se puede conectar a internet
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Conexion completa", "");
  //----

  pinMode(LED, OUTPUT);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);

  pinMode(TEMP, INPUT);

  Serial.begin(115200);
  dht.begin();


  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  unsigned long currentMillis = millis();

}


void loop() {

  tempVieja = temperaturaActual;
  temperaturaActual = dht.readTemperature();

  lecturaSW1 = digitalRead(SW1);
  lecturaSW2 = digitalRead(SW2);

  switch (funcionamiento) {

    case PANTALLA_UNO:
      Serial.println("PRIMERA PANTALLA");

      if (tempVieja != temperaturaActual) {
        display.clearDisplay();

      }

      pantallaUno();

      if (lecturaSW1 == LOW && lecturaSW2 == HIGH) {
        funcionamiento = ESPERA_1;
        currentMillis = millis();
      }
      else {
        alertaTemperatura();
        tiempoBot();
      }
      break;

    case ESPERA_1:
      Serial.println("ESPERA SW1");


      if (lecturaSW1 == HIGH ) {
        funcionamiento = ESPERA_2;
        display.clearDisplay();
        currentMillis = millis();
      }

      tiempoEspera();

      break;

    case ESPERA_2:
      Serial.println("ESPERA SW2");

      if (lecturaSW2 == LOW ) {
        funcionamiento = ESPERA_3;
        display.clearDisplay();
        currentMillis = millis();
      }

      tiempoEspera();


      break;

    case ESPERA_3:
      Serial.println("ESPERA SW2");

      if (lecturaSW2 == HIGH ) {
        funcionamiento = ESPERA_4;
        display.clearDisplay();
        currentMillis = millis();
      }

      tiempoEspera();

      break;

    case ESPERA_4:
      Serial.println("ESPERA SW1");

      if (lecturaSW1 == LOW ) {
        funcionamiento = ESPERA_5;
        display.clearDisplay();
        currentMillis = millis();
      }

      tiempoEspera();


      break;

    case ESPERA_5:
      Serial.println("ESPERA SW1");

      if (lecturaSW1 == HIGH ) {
        funcionamiento = PANTALLA_DOS;
        display.clearDisplay();
      }
      tiempoEspera();

      break;

    case PANTALLA_DOS:
      Serial.println("UMBRAL SUMA O RESTA");


      pantallaDos();


      if (lecturaSW1 == LOW) {
        funcionamiento = RESTA;
      }

      if (lecturaSW2 == LOW) {
        funcionamiento = SUMA;
      }

      break;


    case RESTA:
      Serial.println("RESTA UMBRAL");
      
      if  (valorUmbral < 0) {
        valorUmbral = 0;
      }

      if (lecturaSW1 == HIGH) {
        valorUmbral -= 1;
        display.clearDisplay();
        funcionamiento = PANTALLA_DOS;

      }

      if (lecturaSW2 == LOW) {
        funcionamiento = ESPERA_6;
      }

      break;

    case SUMA:
      Serial.println("SUMA UMBRAL");


      if (lecturaSW2 == HIGH) {
        valorUmbral += 1;
        display.clearDisplay();
        funcionamiento = PANTALLA_DOS;
      }

      if (lecturaSW1 == LOW) {
        funcionamiento = ESPERA_6;
      }

      break;

    case ESPERA_6:
      Serial.println("PANTALLA 1");

      if (lecturaSW1 == HIGH && lecturaSW2 == HIGH) {
        display.clearDisplay();
        funcionamiento = PANTALLA_UNO;
      }
      break;

  }
}

void pantallaUno() {
  display.setCursor(0, 15);
  display.print("Temperatura:");
  display.print(temperaturaActual);
  display.setCursor(0, 35);
  display.print("Umbral:");
  display.print(valorUmbral);
  display.display();
}

void pantallaDos() {

  display.setCursor(0, 25);
  display.print("Umbral:");
  display.println(valorUmbral);
  display.display();
}

void tiempoEspera() {
  unsigned long tiempoActual = millis();
  if (tiempoActual - currentMillis >= INTERVALO) {
    currentMillis = tiempoActual;
    funcionamiento = PANTALLA_UNO;
  }
}

void handleNewMessages(int numNewMessages) {
  Serial.println("Mensaje nuevo");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // inicio de verificacion
    String chat_id = String(bot.messages[i].chat_id);
   /* if (chat_id != CHAT_ID) {  ////si el id no corresponde da error . en caso de que no se quiera comprobar el id se debe sacar esta parte
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    ///fin de verificacion
*/
    // imprime el msj recibido
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/temperatura") {
      bot.sendMessage(chat_id, (String) temperaturaActual, "");
    }

    if (text == "/umbral") {
      bot.sendMessage(chat_id, (String) valorUmbral, "");

    }
  }
}

void tiempoBot() {
  /*  if (lecturaSW1 == LOW && lecturaSW2 == HIGH) {
      funcionamiento = ESPERA_1;
      currentMillis = millis();
      return;
    }
  */
  if (millis() > lastTimeBotRan + botRequestDelay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("Veo los msj nuevos");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void alertaTemperatura() {

  if (valorUmbral > 0) {
    if ((temperaturaActual > valorUmbral) && (flagAlerta == 0)) {
      bot.sendMessage(CHAT_ID, "Se supero el umbral", "");
      flagAlerta = 1;
    }
  }

  //unsigned long tiempoActual2 = millis();
  if (millis() - currentMillis2 >= INTERVALO2) {

    currentMillis2 = millis();
    flagAlerta = !flagAlerta;
  }
}
