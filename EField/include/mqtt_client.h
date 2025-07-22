#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

void mqtt_init(void);
void mqtt_send(float value);
void mqtt_cleanup(void);

#endif // MQTT_CLIENT_H
