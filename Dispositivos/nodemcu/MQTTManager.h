#ifndef _MQTTMANAGER_H_
#define _MQTTMANAGER_H_
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include "macros.h"

//Bibliotecas pro sensor de temperatura e humidade
#include <DHT.h> // including the library of DHT11 temperature and humidity sensor

//Bibliotecas pro MQTT
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

class MQTTManager {
    public:
        MQTTManager();
        ~MQTTManager();
        void conectarBroker(void);
        void initMQTT(void);
        
        void checaTimerCafe();
        void enviaTeH();
    private:
        WiFiClient *client;
        Adafruit_MQTT_Client *mqtt;
        Adafruit_MQTT_Publish *_temp, *_humi;
        Adafruit_MQTT_Subscribe *_porta, *_ac, *_luz, *_tomada;

        //INFORMAÇÕES PARA O DHT11
        DHT *dht;

        template <class T>
        void createSubscribeOrNot(T **object, const char* name, int mode = 1);

        static void portaCallback(char *data, uint16_t len);
        static void luzCallback(char *data, uint16_t len);
        static void tomadaCallback(char *data, uint16_t len);

        static int timer2;
        static bool saida_luz;

        const int tempoProCafe = 1000 * 60 * 2; //2 minutos pra fazer o café
        int contadorCafe = 10; //contador usado na hora de enviar a temperatura

        //INFORMAÇÕES SOBRE O BROKER MQTT
        const static char*  AIO_SERVER;
        const static int AIO_SERVERPORT;
        const static char* AIO_USERNAME;
        const static char* AIO_KEY;
};
#endif
