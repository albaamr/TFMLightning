#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include "bme280.h"

#define I2C_BUS "/dev/i2c-1" // Ruta del bus I2C
#define BME280_I2C_ADDR 0x76 // Dirección del BME280 (verificada con `i2cdetect`)

int8_t i2c_read(uint8_t reg_addr, uint8_t *data, uint16_t len, void *intf_ptr) {
    int fd = *(int *)intf_ptr;
    if (write(fd, &reg_addr, 1) != 1) {
        return -1; // Error al escribir la dirección de registro
    }
    if (read(fd, data, len) != len) {
        return -1; // Error al leer datos
    }
    return 0;
}

int8_t i2c_write(uint8_t reg_addr, const uint8_t *data, uint16_t len, void *intf_ptr) {
    int fd = *(int *)intf_ptr;
    uint8_t buffer[len + 1];
    buffer[0] = reg_addr;
    for (int i = 0; i < len; i++) {
        buffer[i + 1] = data[i];
    }
    if (write(fd, buffer, len + 1) != len + 1) {
        return -1; // Error al escribir datos
    }
    return 0;
}

void delay_us(uint32_t period, void *intf_ptr) {
    usleep(period); // Convertir microsegundos a milisegundos
}

int main() {
    int8_t rslt;
    struct bme280_dev dev;
    struct bme280_data comp_data;

    // Abrir el bus I2C
    int fd = open(I2C_BUS, O_RDWR);
    if (fd < 0) {
        perror("Error al abrir el bus I2C");
        return 1;
    }

    // Configurar el dispositivo I2C
    if (ioctl(fd, I2C_SLAVE, BME280_I2C_ADDR) < 0) {
        perror("Error al configurar la dirección del dispositivo");
        close(fd);
        return 1;
    }

    // Configurar la interfaz del sensor
    dev.intf = BME280_I2C_INTF;
    dev.read = i2c_read;
    dev.write = i2c_write;
    dev.delay_us = delay_us;
    dev.intf_ptr = &fd;

    // Inicializar el sensor
    rslt = bme280_init(&dev);
    if (rslt != BME280_OK) {
        fprintf(stderr, "Error al inicializar el sensor: %d\n", rslt);
        close(fd);
        return 1;
    }

    // Configurar los parámetros del sensor
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;
    dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

    rslt = bme280_set_sensor_settings(BME280_ALL_SETTINGS_SEL, &dev);
    if (rslt != BME280_OK) {
        fprintf(stderr, "Error al configurar el sensor: %d\n", rslt);
        close(fd);
        return 1;
    }

    // Configurar el sensor en modo forzado
    rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
    if (rslt != BME280_OK) {
        fprintf(stderr, "Error al configurar el modo del sensor: %d\n", rslt);
        close(fd);
        return 1;
    }

    // Leer los datos del sensor
    dev.delay_us(40000, dev.intf_ptr); // Esperar para obtener los datos
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
    if (rslt != BME280_OK) {
        fprintf(stderr, "Error al leer los datos del sensor: %d\n", rslt);
        close(fd);
        return 1;
    }

    // Mostrar los datos en la consola
    printf("Temperatura: %.2f °C\n", comp_data.temperature);
    printf("Presión: %.2f hPa\n", comp_data.pressure / 100.0);
    printf("Humedad: %.2f %%\n", comp_data.humidity);

    close(fd); // Cerrar el bus I2C
    return 0;
}
