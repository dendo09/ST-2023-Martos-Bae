
#include "DHT.h"              
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <ESP32Time.h>

#define SCREEN_WIDTH 128 // ancho en pixels
#define SCREEN_HEIGHT 64 // altura en pixels

#define DHTPIN 23    // Digital pin connected to the DHT sensor

#define PIN_BOTON_SUBIR 35 //pines de los botones y sus flags
#define PIN_BOTON_BAJAR 34
bool flagSubir, flagBajar;

// tipo de sensor
#define DHTTYPE DHT11   // DHT 11

#define PANTALLA_MAIN 1    //estados de la maquina
#define ESPERA_CAMBIAR 2
#define PANTALLA_CAMBIAR 3
#define ESPERA_MAIN 4

int estado = 1;  //inicia en el estado 1

// Inicializar el display y el dht
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

//Inicializar un objeto rtc
ESP32Time rtc;

int segs = 0;
int mins = 0;
int hours = 14;

void setup() 
{
  
  Serial.begin(9600);
  dht.begin();
  pinMode(PIN_BOTON_SUBIR, INPUT_PULLUP);    
  pinMode(PIN_BOTON_BAJAR, INPUT_PULLUP);  
  rtc.setTime(segs, mins, hours, 11, 4, 2023); //Setear el rtc con los parametros segs, mins, horas, dias, mes, y año
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  delay(2000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);


}

void loop() 
{
  
  switch(estado)
  {
    
    case PANTALLA_MAIN: //pantalla principal que muestra la temp y la hora   
   
{      
      delay(2000);
      display.setCursor(0, 10);
      float h = dht.readHumidity();
      float t = dht.readTemperature();
  
      display.clearDisplay(); 
      Serial.println(t);
      display.println(t);    
      
      display.print(rtc.getHour(true));  // metodos rtc.get() para obtener el parametro pedido
      display.print(":");                //parametro en true para reloj de 24 hs
      display.print(rtc.getMinute());

      Serial.println(rtc.getHour(true));
      Serial.println(":");                 
      Serial.println(rtc.getMinute());    
       
      display.display();
      Serial.println("1"); //imprimo el estado



        
      if(digitalRead(PIN_BOTON_BAJAR) == LOW && digitalRead(PIN_BOTON_SUBIR) == LOW)// si apreto los botones cambio al estado de espera
      {
        estado = ESPERA_CAMBIAR;
      }
      
    
}
    break;


    case ESPERA_CAMBIAR:  //estado esperando la liberacion de los botones para pasar a la otra pantalla 
         
      if(digitalRead(PIN_BOTON_BAJAR) == HIGH && digitalRead(PIN_BOTON_SUBIR) == HIGH)//si suelto los botones cambio al estado siguiente
      {
        estado = PANTALLA_CAMBIAR;
      }
    break;

      
    case PANTALLA_CAMBIAR: //en esta pantalla se muestra la hora para cambiarse con los botones 

      Serial.println("3");
      display.clearDisplay();
      display.setCursor(0, 10);
      display.print(hours);
      display.print(":");
      display.print(mins);
      display.display();    
      
      if(digitalRead(PIN_BOTON_BAJAR) == LOW) // si pareto el boton, sube el flag para evitar que se cuenten varios clicks
      {
        flagBajar = HIGH;
      }
      if(digitalRead(PIN_BOTON_BAJAR) == HIGH && flagBajar == HIGH) //si suelto el boton, se cuenta un click y cambia el valor de la variable horas
      {
        flagBajar = LOW;                       //reseteo el flag
        hours = hours + 1;
        if (hours > 24){
          hours=0;
        }

      }    

      if(digitalRead(PIN_BOTON_SUBIR) == LOW) //lo mismo con el otro boton
      {
        flagSubir = HIGH;
      }
      if(digitalRead(PIN_BOTON_SUBIR) == HIGH && flagSubir == HIGH)
      {
        flagSubir = LOW;
        mins = mins + 1;
      }    
      
      if(digitalRead(PIN_BOTON_BAJAR) == LOW && digitalRead(PIN_BOTON_SUBIR) == LOW)  //cambio al estado de espera
      {
        estado = ESPERA_MAIN;
      }
    break;


    case ESPERA_MAIN: //estado de espera para volver a la pantalla principal
    
      Serial.println("4");
      if(digitalRead(PIN_BOTON_BAJAR) == HIGH && digitalRead(PIN_BOTON_SUBIR) == HIGH) //vuelvo a la main cuando suelto los dos botones
      {
        estado = PANTALLA_MAIN;
        rtc.setTime(segs, mins, hours, 11, 4, 2023);        // en el cambio de estado, reconfiguro el reloj con los nuevos datos
      }
    
    break;   
  }

}
