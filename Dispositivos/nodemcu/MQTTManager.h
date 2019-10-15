#ifndef _MQTTMANAGER_H_
#define _MQTTMANAGER_H_
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include "macros.h"

//Bibliotecas pro sensor de temperatura e humidade
#include <DHT.h> // including the library of DHT11 temperature and humidity sensor

//Bibliotecas pro MQTT
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//Bibliotecas pro infravermelho
#ifndef ARDUINO
#define ARDUINO
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRsend.h>

class MQTTManager {
    public:
        MQTTManager();
        ~MQTTManager();
        void conectarBroker(void);
        void initMQTT(void);
        
        void checaTimerCafe();
        void checaPresenca();
        void enviaTeH();
    private:
        WiFiClient *client;
        Adafruit_MQTT_Client *mqtt;
        Adafruit_MQTT_Publish *_temp, *_humi, *_presenca;
        Adafruit_MQTT_Subscribe *_porta, *_ac, *_luz, *_tomada;

        //IRsend *irSend;

        //INFORMAÇÕES PARA O DHT11
        DHT *dht;

        template <class T>
        void createSubscribeOrNot(T **object, const char* name, int mode = 1);

        static void portaCallback(char *data, uint16_t len);
        static void luzCallback(char *data, uint16_t len);
        static void tomadaCallback(char *data, uint16_t len);
        static void ac_Callback(char *data, uint16_t len);

        static int timer2;
        static bool saida_luz;

        // Variables will change:

        int presencaState = 0;         // current state of the sensor
        int lastpresencaState = 0;     // previous state of the sensor

        const int tempoProCafe = 1000 * 60 * 2; //2 minutos pra fazer o café
        int contadorCafe = 10; //contador usado na hora de enviar a temperatura

        //INFORMAÇÕES SOBRE O BROKER MQTT
        const static char*  AIO_SERVER;
        const static int AIO_SERVERPORT;
        const static char* AIO_USERNAME;
        const static char* AIO_KEY;
};
#endif
