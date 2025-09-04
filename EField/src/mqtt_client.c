#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "MQTTClient.h"
#include "mqtt_client.h"

// Configuración MQTT
#define ADDRESS     "tcp://broker.hivemq.com:1883"
#define CLIENTID    "RaspiFieldSensor0"
#define TOPIC       "eField/reading"
#define ALERT_TOPIC "alert/electrostatic"
#define QOS         1
#define TIMEOUT     10000L

MQTTClient client;
static pthread_mutex_t mqtt_mutex = PTHREAD_MUTEX_INITIALIZER;

void mqtt_init(void) {
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    int rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "Error al conectar con el broker MQTT. Código: %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Conectado a MQTT broker en %s\n", ADDRESS);
}

void mqtt_send(float value) {
    char payload[50];
    snprintf(payload, sizeof(payload), "%.3f", value);

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload = payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    printf("[TRACE] Enviando mensaje: %s\n", payload);

    pthread_mutex_lock(&mqtt_mutex);
    int rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "[ERROR] Error al publicar el mensaje (codigo %d)\n", rc);
    } else {
        MQTTClient_waitForCompletion(client, token, TIMEOUT);
    }
    pthread_mutex_unlock(&mqtt_mutex);
}

int mqtt_send_alert_json(const char* json) {
    if (!json) return -1;

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload = (void*)json;
    pubmsg.payloadlen = (int)strlen(json);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    int rc = MQTTClient_publishMessage(client, ALERT_TOPIC, &pubmsg, &token);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "Error al publicar alerta en %s (codigo %d)\n", ALERT_TOPIC, rc);
        return rc;
    }
    MQTTClient_waitForCompletion(client, token, TIMEOUT);
    return 0;
}

void mqtt_cleanup(void) {
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}
