#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include "mqtt_as3935.h"

// --- Configuración MQTT (ajústala si lo necesitas) ---
#define ADDRESS         "tcp://broker.hivemq.com:1883"
#define CLIENTID        "RaspiAS3935_0"
#define TOPIC_LIGHTNING "ThunderSystem/alert/lightning"
#define TOPIC_NOISE     "ThunderSystem/as3935/noise"
#define TOPIC_INTERF    "ThunderSystem/as3935/interference"
#define QOS             1
#define TIMEOUT         10000L
// ------------------------------------------------------

static MQTTClient client;
static int mqtt_ready = 0;

static void publish_simple(const char *topic, const char *payload) {
    if (!mqtt_ready) return;

    MQTTClient_message msg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    msg.payload = (void*)payload;
    msg.payloadlen = (int)strlen(payload);
    msg.qos = QOS;
    msg.retained = 0;

    int rc = MQTTClient_publishMessage(client, topic, &msg, &token);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "[MQTT] Error al publicar en %s (rc=%d)\n", topic, rc);
        return;
    }
    MQTTClient_waitForCompletion(client, token, TIMEOUT);
}

void mqtt_as3935_init(void) {
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    int rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "[MQTT] No se pudo conectar al broker (%s). rc=%d\n", ADDRESS, rc);
        exit(EXIT_FAILURE);
    }
    mqtt_ready = 1;
    printf("[MQTT] Conectado a %s\n", ADDRESS);
    printf("[MQTT] Topics: %s | %s | %s\n", TOPIC_LIGHTNING, TOPIC_NOISE, TOPIC_INTERF);
}

void mqtt_as3935_publish_lightning(int distance_km) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", distance_km);
    publish_simple(TOPIC_LIGHTNING, buf);
}

void mqtt_as3935_publish_noise(void) {
    publish_simple(TOPIC_NOISE, "noise");
}

void mqtt_as3935_publish_interference(void) {
    publish_simple(TOPIC_INTERF, "interference");
}

void mqtt_as3935_cleanup(void) {
    if (!mqtt_ready) return;
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    mqtt_ready = 0;
}
