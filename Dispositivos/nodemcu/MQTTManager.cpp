#include "MQTTManager.h"
#include <stdlib.h>
#include <string.h>

#ifndef MQTT_MODE
#define MQTT_MODE 
#define MQTT_SUBSCRIBE 1
#define MQTT_PUBLISH 0
#endif

bool MQTTManager::saida_luz = HIGH;
int MQTTManager::timer2 = 0;
const char*  MQTTManager::AIO_SERVER =  "192.168.0.5";
const int MQTTManager::AIO_SERVERPORT = 1883;
const char* MQTTManager::AIO_USERNAME = "";
const char* MQTTManager::AIO_KEY = "";



MQTTManager::MQTTManager(){
    client = new WiFiClient();
    mqtt = new Adafruit_MQTT_Client(client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
    dht = new DHT(dht_dpin, DHTTYPE);
    dht->begin();

    //INFORMAÇÕES PARA O IR
    const uint16_t kIrLed = 4;     // ESP8266 GPIO pin to use. Recommended: 4 (D2).
    irSend = new irSend(kIrLed);         // Set the GPIO to be used to sending the message.
    irSend->begin();
    
    createSubscribeOrNot(&_humi, "/b26/umidade", MQTT_PUBLISH);
    createSubscribeOrNot(&_porta, "/b26/porta", MQTT_SUBSCRIBE);
    createSubscribeOrNot(&_ac, "/b26/ac", MQTT_SUBSCRIBE);
    createSubscribeOrNot(&_luz, "/b26/luz", MQTT_SUBSCRIBE);
    createSubscribeOrNot(&_tomada, "/b26/tomada", MQTT_SUBSCRIBE);
    createSubscribeOrNot(&_temp, "/b26/temperatura", MQTT_PUBLISH);
    createSubscribeOrNot(&_presenca, "/b26/presenca", MQTT_PUBLISH);
    Serial.print("sobrevivi");
}

MQTTManager::~MQTTManager(){
    delete client, mqtt, _temp, _humi, _porta, _ac, _luz, _tomada, _presenca;
}

void MQTTManager::conectarBroker() {
    int8_t ret;

    if (mqtt->connected()) {
        mqtt->processPackets(5000);
        delay(100);
        mqtt->ping();
        return;
    }

    Serial.println("Conectando-se ao broker mqtt...");

    uint8_t num_tentativas = 3;

    while ((ret = mqtt->connect()) != 0) {
        Serial.println(mqtt->connectErrorString(ret));
        Serial.println("Falha ao se conectar. Tentando se reconectar em 10 segundos.");
        mqtt->disconnect();
        delay(10000);
        num_tentativas--;
        if (num_tentativas == 0) {
            Serial.println("Seu ESP será resetado.");
            ESP.restart();
        }
    } //Depois que esgotar o número de tentivas, reseta o NodeMCU
    Serial.println("Conectado ao broker com sucesso!");
}

void MQTTManager::initMQTT() {
    _porta->setCallback(MQTTManager::portaCallback);
    mqtt->subscribe(_porta);
    _ac->setCallback(MQTTManager::ac_Callback);
    mqtt->subscribe(_ac);
    _luz->setCallback(MQTTManager::luzCallback);
    mqtt->subscribe(_luz);
    _tomada->setCallback(MQTTManager::tomadaCallback);
    mqtt->subscribe(_tomada);
}

template <class T>
void MQTTManager::createSubscribeOrNot(T** object, const char* path,int mode) {
    if(mode == 0){
        Adafruit_MQTT_Publish **publish = reinterpret_cast<Adafruit_MQTT_Publish **>(object);
        *publish = (Adafruit_MQTT_Publish *) malloc(sizeof(Adafruit_MQTT_Publish));
        **publish = Adafruit_MQTT_Publish(mqtt, path, MQTT_QOS_1);
    }else if(mode == 1) {
        Adafruit_MQTT_Subscribe **subscribe = reinterpret_cast<Adafruit_MQTT_Subscribe **>(object);
        *subscribe = (Adafruit_MQTT_Subscribe *) malloc(sizeof(Adafruit_MQTT_Subscribe));
        **subscribe = Adafruit_MQTT_Subscribe(mqtt, path, MQTT_QOS_1);
    }
}

void MQTTManager::checaTimerCafe() {
    if(timer2-millis() > tempoProCafe){
        digitalWrite(PINO_TOMADA,HIGH);
        timer2=0;
    }
}

void MQTTManager::checaPresenca() {
    presencaState = digitalRead(PINO_SENSORES);
    Serial.println(presencaState);

    // compare the sensor state to its previous state
    if (presencaState != lastpresencaState) {
        // if the state has changed, increment the counter
        if (presencaState == HIGH) {
            Serial.println("O sensor foi ativado");
            if (! _presenca->publish(presencaState)) {  
                Serial.println("Falha ao enviar o estado do sensor."); 
            }  
            else {
            Serial.println("Estado do sensor enviado!"); 
            }
        } 
        
        else {
        // if the current state is LOW then the button went from on to off:
        Serial.println("O sensor voltou a posicao normal");
        }

    lastpresencaState = presencaState;
    }
}

void MQTTManager::enviaTeH() {
    if( contadorCafe==10 ) {
        float h = dht->readHumidity();
        float t = dht->readTemperature();

        if (! _temp->publish(t)) {  
            Serial.println("Falha ao enviar a temperatura."); 
        }  
        else {
          Serial.println("Temperatura enviada!"); Serial.println(t);
        }
        
        if (! _humi->publish(h)) {  
            Serial.println("Falha ao enviar a umidade."); 
        } 
        else {
          Serial.println("Umidade enviada!"); Serial.println(h);
        }
        

        contadorCafe = 0;

      } //envia temperatura
      else{ contadorCafe++; }
}

void MQTTManager::portaCallback(char *data, uint16_t len) {
    String comando = data;

    Serial.print("Comando para porta: ");
    Serial.println(comando);

    if (comando == "1") {
        digitalWrite(PINO_PORTA, LOW);
        delay(500);
        digitalWrite(PINO_PORTA, HIGH);
    }
}

void MQTTManager::ac_Callback(char *data, uint16_t len) {
    String comando = data;

    Serial.print("Comando para o ar: ");
    Serial.println(comando);

    if (comando == "1") {
      uint16_t liga_ac[59] = {8450,4200, 600,1550, 600,500, 600,500, 600,500, 550,1600, 600,500, 600,500, 600,500, 550,550, 550,500, 600,500, 600,500, 600,500, 550,550, 550,550, 550,500, 600,500, 600,500, 600,1550, 600,1600, 550,550, 550,550, 550,1600, 600,500, 550,550, 550,1600, 600,500, 600,1600, 550};  // LG 8800325
      irSend->sendRaw(liga_ac, 59, 38);  // Send a raw data capture at 38kHz.
    }
    else if (comando == "0"){
      uint16_t desliga_ac[59] = {8450,4200, 600,1600, 550,500, 600,500, 600,500, 600,1600, 550,500, 600,500, 600,500, 600,1600, 550,1600, 600,500, 550,550, 550,500, 600,500, 600,500, 600,500, 550,550, 550,550, 550,500, 600,500, 600,500, 600,1550, 600,500, 600,1600, 550,550, 550,500, 600,500, 600,1600, 550};  // LG 88C0051
      irSend->sendRaw(desliga_ac, 59, 38);  // Send a raw data capture at 38kHz.
    }
}
void MQTTManager::luzCallback(char *data, uint16_t len) {
    String comando = data;
    Serial.print("Comando para a iluminação: ");
    Serial.println(comando);

    if (comando == "1" and digitalRead(PINO_SENSOR_DE_TENSAO) == 0) {
        saida_luz = !saida_luz;
        digitalWrite(PINO_LUZ, saida_luz);
    }
    else if (comando == "0" and digitalRead(PINO_SENSOR_DE_TENSAO) == 1) {
        saida_luz = !saida_luz;
        digitalWrite(PINO_LUZ, saida_luz);
    }
}
void MQTTManager::tomadaCallback(char *data, uint16_t len) {
    String comando = data;
    Serial.print("Comando para a tomada: ");
    Serial.println(comando);

    if (comando == "1") {
        timer2 = millis();
        digitalWrite(PINO_TOMADA, LOW);
    }
}
