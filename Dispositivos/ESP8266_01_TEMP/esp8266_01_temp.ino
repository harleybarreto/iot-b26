#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//Bibliotecas pro WifiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

//Bibliotecas pro MQTT
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//Bibliotecas pro sensor de temperatura e humidade
#include "DHT.h"        // including the library of DHT11 temperature and humidity sensor
#define DHTTYPE DHT11   // DHT 11

#define dht_dpin 0               //GPIO 0

//INFORMAÇÕES PARA O DHT11
DHT dht(dht_dpin, DHTTYPE);

//INFORMAÇÕES SOBRE O BROKER MQTT
#define AIO_SERVER      "192.168.0.5"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    ""
#define AIO_KEY         ""

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish   _temp    = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/b26/temperatura", MQTT_QOS_1);
Adafruit_MQTT_Publish   _humi    = Adafruit_MQTT_Publish  (&mqtt, AIO_USERNAME "/b26/umidade", MQTT_QOS_1);

void conectar_wifi();
void conectar_broker();
void envia_TeH();

void setup() {

    //Serial.begin(115200);

    dht.begin();

    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

    inicia_pinos();
    conectar_wifi();
    initMQTT();
}

void loop() {

  if(WiFi.status()== WL_CONNECTED){ // Se o Wi-Fi estiver conectado...
    conectar_broker();              //Testa se já está conectado e conecta caso não esteja
    envia_TeH();
  }

  else{  conectar_wifi();  }

delay(10000); //Trocar por deep sleep mode para poupar bateria
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

//Envia temperatura e umidade
void envia_TeH() {

        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (! _temp.publish(t)) {  
            Serial.println("Falha ao enviar a temperatura."); 
        }
        else{
            Serial.println("Temperatura enviada!");
        }

        if (! _humi.publish(h)) {  
            Serial.println("Falha ao enviar a umidade."); 
        }
        else{
            Serial.println("Umidade enviada!");
        }
        
}


