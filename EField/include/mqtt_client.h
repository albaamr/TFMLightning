#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

void mqtt_init(void);
void mqtt_send(float value);
int mqtt_send_alert_json(const char* json);
void mqtt_cleanup(void);

#endif // MQTT_CLIENT_H
