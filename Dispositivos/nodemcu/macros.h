#ifndef _MACROS_H_
#define _MACROS_H_
#define DHTTYPE DHT11 // DHT 11

#define PINO_SENSORES 5          //Pino D1 \
                                 //Pino D2 é o LED IR
#define dht_dpin 0               //Pino D3 \
                                 //Pino D4 não está sendo usado
#define PINO_LUZ 14              //Pino D5
#define PINO_TOMADA 12           //Pino D6
#define PINO_PORTA 13            //Pino D7
#define PINO_SENSOR_DE_TENSAO 15 //Pino D8

//INFORMAÇÕES SOBRE O BROKER MQTT
#define AIO_SERVER "192.168.0.5"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME ""
#define AIO_KEY ""
#endif