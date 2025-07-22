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

int main(void) {
    if (openI2CBus("/dev/i2c-1") == -1) return EXIT_FAILURE;
    setI2CSlave(0x48);

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

    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    while (1) {
        float voltage = readVoltage(0);

        struct timespec current_time;
        clock_gettime(CLOCK_REALTIME, &current_time);
        double t_rel = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1e9;

        // Guardar en CSV
        fprintf(csv_file, "%.3f,%.3f\n", t_rel, voltage);
        fflush(csv_file);

        buffer_push(voltage);
        // Get current time
        /*time_t rawtime;
        struct tm *timeinfo;
        char timestamp[20]; // Buffer for timestamp (YYYY-MM-DD HH:MM:SS)
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

        printf("[%s] Lectura %d: Voltaje leido: %.2f\n", timestamp, ++counter, voltage);*/
    }

    fclose(csv_file);
    return EXIT_SUCCESS;
}
