#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "ads1115_rpi.h"
#include "mqtt_client.h"

#define BUFFER_SIZE 500
float adc_buffer[BUFFER_SIZE];
volatile int head = 0;
volatile int tail = 0;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

FILE *csv_file = NULL;
char csv_filename[64];

static float V_HIGH_THR = 4.0f;   // V
static float V_LOW_THR  = 1.0f;   // V

static void load_env_thresholds(void) {
    const char* sH = getenv("V_HIGH_THR");
    const char* sL = getenv("V_LOW_THR");
    if (sH) V_HIGH_THR = strtof(sH, NULL);
    if (sL) V_LOW_THR  = strtof(sL, NULL);
    fprintf(stdout, "[CFG] V_HIGH_THR=%.3f V, V_LOW_THR=%.3f V\n", V_HIGH_THR, V_LOW_THR);
}

void buffer_push(float value) {
    pthread_mutex_lock(&buffer_mutex);
    adc_buffer[head] = value;
    head = (head + 1) % BUFFER_SIZE;
    if (head == tail) {
        // Overflow: avanzar tail para no bloquear
        tail = (tail + 1) % BUFFER_SIZE;
    }
    pthread_mutex_unlock(&buffer_mutex);
}

int buffer_pop(float *value) {
    pthread_mutex_lock(&buffer_mutex);
    if (head == tail) {
        pthread_mutex_unlock(&buffer_mutex);
        return 0; // Buffer vacío
    }
    *value = adc_buffer[tail];
    tail = (tail + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&buffer_mutex);
    return 1;
}

void *mqtt_task(void *arg) {
    mqtt_init();

    while (1) {
        float value;
        if (buffer_pop(&value)) {
            mqtt_send(value);
            // Opcional: sleep breve para no saturar red
            usleep(5000);
        } else {
            // Esperar si no hay datos
            usleep(1000);
        }
    }

    mqtt_cleanup();
    return NULL;
}

void generate_csv_filename() {
    int i = 1;
    while (1) {
        snprintf(csv_filename, sizeof(csv_filename), "datos_adc_%d.csv", i);
        FILE *f = fopen(csv_filename, "r");
        if (f == NULL) {
            break; // Archivo no existe → usar este
        }
        fclose(f);
        i++;
    }
}

static inline void wallclock(char* buf, size_t n) {
    time_t rawtime = time(NULL);
    struct tm *timeinfo = localtime(&rawtime);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", timeinfo);
}

int main(void) {
    if (openI2CBus("/dev/i2c-1") == -1) return EXIT_FAILURE;
    setI2CSlave(0x48);

    load_env_thresholds();

    generate_csv_filename();
    csv_file = fopen(csv_filename, "w");
    if (!csv_file) {
        perror("Error creando archivo CSV");
        return EXIT_FAILURE;
    }
    fprintf(csv_file, "timestamp,voltaje\n");
    printf("[INFO] Guardando CSV en: %s\n", csv_filename);

    pthread_t mqtt_thread;
    pthread_create(&mqtt_thread, NULL, mqtt_task, NULL);

    while (1) {
        float voltage = readVoltage(0);
        
        char timestamp[32];
        wallclock(timestamp, sizeof(timestamp));
        // Guardar en CSV
        fprintf(csv_file, "%s,%.3f\n", timestamp, voltage);
        fflush(csv_file);

        buffer_push(voltage);

        if (voltage >= V_HIGH_THR || voltage <= V_LOW_THR) {
            char json[256];
            snprintf(json, sizeof(json),
                     "{\"timestamp\":\"%s\",\"v\":%.5f,"
                     "\"v_high_thr\":%.5f,\"v_low_thr\":%.5f,"
                     "\"trigger\":\"%s\"}",
                     timestamp,
                     voltage,
                     V_HIGH_THR,
                     V_LOW_THR,
                     (voltage >= V_HIGH_THR ? "HIGH" : "LOW"));

            mqtt_send_alert_json(json);
        }
    }

    fclose(csv_file);
    return EXIT_SUCCESS;
}
