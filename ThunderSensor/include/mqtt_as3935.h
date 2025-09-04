#ifndef MQTT_AS3935_H
#define MQTT_AS3935_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa el cliente MQTT para eventos AS3935.
 * Sale del proceso con EXIT_FAILURE si no conecta.
 */
void mqtt_as3935_init(void);

/**
 * Publica un rayo en el topic de rayos.
 * @param distance_km Distancia en km, o -1 si desconocida (0x3F).
 */
void mqtt_as3935_publish_lightning(int distance_km);

/**
 * Publica un evento de ruido (INT_NH) en su topic dedicado.
 */
void mqtt_as3935_publish_noise(void);

/**
 * Publica un evento de interferencia (INT_D) en su topic dedicado.
 */
void mqtt_as3935_publish_interference(void);

/**
 * Desconecta y destruye el cliente MQTT.
 */
void mqtt_as3935_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // MQTT_AS3935_H
