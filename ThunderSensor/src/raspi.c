/**
 * @file raspi.c
 * @brief Raspberry initialisation and interrupt handling
 *
 * @author Alba Moreno Ramos
 * @date 
 */

 //Cambiar perrors a inglés
#include "raspi.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "AS3935.h"

#define GPIO_CHIP "/dev/gpiochip0"

int n = 0;
int r = 0;

void systemInit(struct gpiod_chip **chip, struct gpiod_line **line)
{    
    uint8_t mode = SPI_MODE_1;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED;

    spi_fd = open(SPI_BUS, O_RDWR); //ENCAPSULAR SPI_FD!!!! ES DECIR, NO PONER COMO EXTERN
    if (spi_fd < 0) {
        perror("Error al abrir el bus SPI");
        return;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Error al configurar el modo SPI");
        return;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        perror("Error al configurar bits por palabra en SPI");
        return;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Error al configurar la velocidad SPI");
        return;
    }

    *chip = gpiod_chip_open(GPIO_CHIP);
    if (!(*chip)) {
        perror("Error al abrir el chip GPIO");
        return;
    }

    *line = gpiod_chip_get_line(*chip, GPIO_IRQ);
    if (!(*line)) {
        perror("Error al obtener la línea GPIO");
        gpiod_chip_close(*chip);
        return;
    }

    int ret = gpiod_line_request_rising_edge_events(*line, "as3935_irq");
    if (ret < 0) {
        perror("Error al configurar la línea GPIO");
        gpiod_chip_close(*chip);
        close(spi_fd);
        return;
    }
    usleep(DELAY_200MS);
}

void handle_interrupt(struct gpiod_line *line) {
    int value = gpiod_line_get_value(line);
    if (value < 0) {
        perror("Error al leer el valor del GPIO");
        return;
    }

    if (value == 1) {
        
        time_t t;
        struct tm *tm_info;
        char buffer[20];

        uint8_t event = spi_read_register(CONFIG_REG_3) & 0x0F;
        do {
            usleep(DELAY_1MS); 
        } while (gpiod_line_get_value(line) == 1);


        time(&t);
        tm_info = localtime(&t);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

        printf("⚡ Interrupción detectada: "); 
        if (log_file) {
            switch (event) { 
                case 0x01:
                    printf("Ruido demasiado alto ⚠️ (INT_NH) - Hora: %s\n", buffer);
                    fprintf(log_file, "%s - Ruido alto (INT_NH)\n", buffer);
                    break;
                case 0x04:
                    n++;
                    printf("Interferencia detectada 🌩 (INT_D). Evento %i - Hora: %s\n",n,buffer);
                    fprintf(log_file, "%s - Interferencia (INT_D), Evento %i\n", buffer, n);
                    break;
                case 0x08:
                    r++;

                    uint8_t distancia_raw = spi_read_register(0x07) & AS3935_REG_MASK;
                    fprintf(log_file, "%s - Rayo detectado (INT_L), Rayo %i, Distancia: %d km\n", buffer, r, distancia_raw == 0x3F ? -1 : distancia_raw);
                    printf("¡Rayo %i detectado! ⚡ (INT_L) - Hora: %s\n", r, buffer);

                    if (distancia_raw == 0x3F) { // 0x3F significa "fuera de rango"
                        printf("Distancia: Fuera de rango (>40 km)\n");
                    } else {
                        printf("Distancia estimada: %d km\n", distancia_raw);
                    }
                    
                    break;
                default:
                    n++;
                    printf("Evento %i desconocido (0x%02X)\n", n, event);
                    fprintf(log_file, "%s - Evento desconocido (0x%02X), Evento %i\n", buffer, event, n);
            }
            fflush(log_file);
        }
    }
}