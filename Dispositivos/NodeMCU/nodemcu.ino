#include <FS.h>                   //this needs to be first, or it all crashes and burns... //PARA TER IP FIXO COM O WiFiManager

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//Bibliotecas pro WifiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

//Bibliotecas pro infravermelho

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRsend.h>


//Bibliotecas pro MQTT
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//Bibliotecas pro sensor de temperatura e humidade
#include "DHT.h"        // including the library of DHT11 temperature and humidity sensor
#define DHTTYPE DHT11   // DHT 11

#define PINO_SENSORES 5          //Pino D1
                                 //Pino D2 é o LED IR
#define dht_dpin 0               //Pino D3
                                 //Pino D4 não está sendo usado
#define PINO_LUZ 14              //Pino D5
#define PINO_TOMADA 12           //Pino D6
#define PINO_PORTA 13            //Pino D7
#define PINO_SENSOR_DE_TENSAO 15 //Pino D8


//INFORMAÇÕES PARA O IR
//const uint16_t kIrLed = 4;     // ESP8266 GPIO pin to use. Recommended: 4 (D2).
//IRsend irsend(kIrLed);         // Set the GPIO to be used to sending the message.

//INFORMAÇÕES PARA O DHT11
DHT dht(dht_dpin, DHTTYPE);

//INFORMAÇÕES SOBRE O BROKER MQTT
#define AIO_SERVER      "192.168.0.5"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    ""
#define AIO_KEY         ""

int i=0;                                       //contador usado na hora de enviar a temperatura
int tempo_de_espera_do_sensor = 1000 * 60 * 2; //2 minutos
int tempo_pro_cafe = 1000 * 60 * 2  ;           //2 minutos pra fazer o café
int timer1 = 0;                                //Timer sensor de presença
int timer2 = 0;                                //Timer da tomada
bool saida_luz = HIGH;                         //variável usada pra indicar a saída que vai pro relé das lampâdas

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish   _temp    = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/b26/temperatura", MQTT_QOS_1);
Adafruit_MQTT_Publish   _humi    = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/b26/umidade", MQTT_QOS_1);
Adafruit_MQTT_Subscribe _porta   = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/b26/porta", MQTT_QOS_1);
Adafruit_MQTT_Subscribe _ac      = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/b26/ac", MQTT_QOS_1);
Adafruit_MQTT_Subscribe _luz     = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/b26/luz", MQTT_QOS_1);
Adafruit_MQTT_Subscribe _tomada  = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/b26/tomada", MQTT_QOS_1);

void inicia_pinos();
void conectar_wifi();
void initMQTT();
void conectar_broker();
void envia_TeH();
//void checa_sensor_de_presenca();
void checa_timer_cafe();

void setup() {

    //Serial.begin(115200);

    //irsend.begin();
    dht.begin();

    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

    inicia_pinos();
    conectar_wifi();
    initMQTT();
}

void loop() {

//  checa_sensor_de_presenca();
  checa_timer_cafe();

  if(WiFi.status()== WL_CONNECTED){ // Se o Wi-Fi estiver conectado...
    conectar_broker();              //Testa se já está conectado e conecta caso não esteja
    envia_TeH();
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

  digitalWrite(PINO_LUZ,saida_luz);
  digitalWrite(PINO_TOMADA,HIGH);
  digitalWrite(PINO_PORTA,HIGH);
}

//Conexão com a rede Wi-Fi
void conectar_wifi(){
    WiFiManager wifiManager;
    wifiManager.autoConnect("AutoConnectAP");
    Serial.println("connected...yeey :)");
}

/* Conexão com o broker e também servirá para reestabelecer a conexão caso caia */
void conectar_broker() {
  int8_t ret;

  if (mqtt.connected()) {
    mqtt.processPackets(5000);
    delay(100);
    mqtt.ping();
    return;
  }

  Serial.println("Conectando-se ao broker mqtt...");

  uint8_t num_tentativas = 3;

  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Falha ao se conectar. Tentando se reconectar em 10 segundos.");
    mqtt.disconnect();
    delay(10000);
    num_tentativas--;
    if (num_tentativas == 0) {
      Serial.println("Seu ESP será resetado.");
      ESP.restart();
    } //Depois que esgotar o número de tentivas, reseta o NodeMCU

  }

  Serial.println("Conectado ao broker com sucesso.");
}
/* Configuração da conexão MQTT */
void initMQTT() {
  _porta.setCallback(porta_callback);
  mqtt.subscribe(&_porta);
//  _ac.setCallback(ac_callback);
//  mqtt.subscribe(&_ac);
  _luz.setCallback(luz_callback);
  mqtt.subscribe(&_luz);
  _tomada.setCallback(tomada_callback);
  mqtt.subscribe(&_tomada);
}

/* callback responsavel por tratar o feed da porta */                            //Aciona o pino da fechadura por meio segundo.
void porta_callback(char *data, uint16_t len) {
  String comando= data;

  Serial.print("Comando para porta: ");
  Serial.println(comando);

  if(comando=="1"){
    digitalWrite(PINO_PORTA,LOW);
    delay(500);
    digitalWrite(PINO_PORTA,HIGH);
  }
}

/* callback responsavel por tratar o feed do ac */                               //Manda os comandos IR de ligar e desligar.
/*
void ac_callback(char *data, uint16_t len) {
  String comando= data;

  Serial.print("Comando para o AC: ");
  Serial.println(comando);

  if(comando=="1"){
    uint16_t liga_ac[59] = {8450,4200, 600,1550, 600,500, 600,500, 600,500, 550,1600, 600,500, 600,500, 600,500, 550,550, 550,500, 600,500, 600,500, 600,500, 550,550, 550,550, 550,500, 600,500, 600,500, 600,1550, 600,1600, 550,550, 550,550, 550,1600, 600,500, 550,550, 550,1600, 600,500, 600,1600, 550};  // LG 8800325
    irsend.sendRaw(liga_ac, 59, 38);  // Send a raw data capture at 38kHz.
  }
  else if(comando=="0"){
    uint16_t desliga_ac[59] = {8450,4200, 600,1600, 550,500, 600,500, 600,500, 600,1600, 550,500, 600,500, 600,500, 600,1600, 550,1600, 600,500, 550,550, 550,500, 600,500, 600,500, 600,500, 550,550, 550,550, 550,500, 600,500, 600,500, 600,1550, 600,500, 600,1600, 550,550, 550,500, 600,500, 600,1600, 550};  // LG 88C0051
    irsend.sendRaw(desliga_ac, 59, 38);  // Send a raw data capture at 38kHz.
  }
}
*/

/* callback responsavel por tratar o feed da iluminação */
void luz_callback(char *data, uint16_t len) {
  String comando= data;
  Serial.print("Comando para a iluminação: ");
  Serial.println(comando);

  if(comando=="1" and digitalRead(PINO_SENSOR_DE_TENSAO)==0){
    saida_luz= !saida_luz;
    digitalWrite(PINO_LUZ,saida_luz);
  }
  else if(comando=="0" and digitalRead(PINO_SENSOR_DE_TENSAO)==1){
    saida_luz= !saida_luz;
    digitalWrite(PINO_LUZ,saida_luz);  }

}

/* callback responsavel por tratar o feed da tomada controlada */                //FALTA COMPLETAR. Qual tempo que a tomada precisa ficar ligada?
void tomada_callback(char *data, uint16_t len) {
  String comando= data;
  Serial.print("Comando para a tomada: ");
  Serial.println(comando);

  if(comando=="1"){
    timer2 = millis();
    digitalWrite(PINO_TOMADA,LOW);
  }
}

//Checa o timer do café
void checa_timer_cafe(){
    if(timer2-millis() > tempo_pro_cafe){
    digitalWrite(PINO_TOMADA,HIGH);
    timer2=0;
  }
}

//Envia temperatura e umidade
void envia_TeH() {


      if( i==10 ){

        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (! _temp.publish(t)) {  Serial.println("Falha ao enviar a temperatura."); }
        Serial.println("Temperatura enviada!"); Serial.println(t);
        if (! _humi.publish(h)) {  Serial.println("Falha ao enviar a umidade."); }
        Serial.println("Umidade enviada!"); Serial.println(h);

        i=0;

      } //envia temperatura
      else{ i++; }
}

/*
void checa_sensor_de_presenca(){

  if (digitalRead(PINO_SENSORES) == LOW){
    timer1 = millis(); //Zera o timer
    Serial.println("Há movimento na sala");
  }

  if(millis() - timer1 > tempo_de_espera_do_sensor){ //Hora de desligar AC e iluminação
    Serial.println("Sem movimento na sala, desligando AC e LUZ");

    uint16_t desliga_ac[59] = {8450,4200, 600,1600, 550,500, 600,500, 600,500, 600,1600, 550,500, 600,500, 600,500, 600,1600, 550,1600, 600,500, 550,550, 550,500, 600,500, 600,500, 600,500, 550,550, 550,550, 550,500, 600,500, 600,500, 600,1550, 600,500, 600,1600, 550,550, 550,500, 600,500, 600,1600, 550};  // LG 88C0051
    irsend.sendRaw(desliga_ac, 59, 38);  // Send a raw data capture at 38kHz.

     if(digitalRead(PINO_SENSOR_DE_TENSAO)){ //Se ler 1 do sensor de tensão, desliga caso contrário já está desligada
      saida_luz= !saida_luz;
      digitalWrite(PINO_LUZ,saida_luz);    }

  }
}
*/
