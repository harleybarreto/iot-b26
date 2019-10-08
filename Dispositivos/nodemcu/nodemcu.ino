// #include <FS.h>                   //this needs to be first, or it all crashes and burns... //PARA TER IP FIXO COM O WiFiManager
#include "MQTTManager.h"

//Bibliotecas pro WifiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

//Bibliotecas pro infravermelho

#ifndef ARDUINO
#define ARDUINO
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRsend.h>

//INFORMAÇÕES PARA O IR
const uint16_t kIrLed = 4;     // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);         // Set the GPIO to be used to sending the message.

void inicia_pinos();
void conectar_wifi();

MQTTManager *manager = NULL;

void setup() {

    //Serial.begin(115200);
    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

    irsend.begin();
    //Serial.begin(9600);

    inicia_pinos();
    conectar_wifi();
    manager = new MQTTManager();
    manager->initMQTT();

}
void loop() {  
  manager->checaPresenca();
  manager->checaTimerCafe();

  if(WiFi.status()== WL_CONNECTED){ // Se o Wi-Fi estiver conectado...
    manager->conectarBroker();              //Testa se já está conectado e conecta caso não esteja
    manager->enviaTeH();
  }

  else{  conectar_wifi();  }
}

//Inicia as saídas do relé
void inicia_pinos(){
  pinMode(PINO_LUZ, OUTPUT);
  pinMode(PINO_TOMADA, OUTPUT);
  pinMode(PINO_PORTA, OUTPUT);
  pinMode(PINO_SENSOR_DE_TENSAO,INPUT);
  pinMode(PINO_SENSORES,INPUT);

  digitalWrite(PINO_LUZ,HIGH);
  digitalWrite(PINO_TOMADA,HIGH);
  digitalWrite(PINO_PORTA,HIGH);
}

//Conexão com a rede Wi-Fi
void conectar_wifi(){
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yeey :)");
}
