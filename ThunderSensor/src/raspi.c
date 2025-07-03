/**
 * @file raspi.c
 * @brief Raspberry initialisation and interrupt handling
 *
 * @author Alba Moreno Ramos
 * @date 21-05-2025
 */

#include "raspi.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "AS3935.h"

void log_timestamp(char *buffer, size_t size) {
    time_t t = time(NULL);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", localtime(&t));
}

int systemInit(struct SystemState *state, struct gpiod_chip **chip, struct gpiod_line **line)
{    
    uint8_t mode = SPI_MODE_1;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED;

    state->spi_fd = open(SPI_BUS, O_RDWR);
    if (state->spi_fd < 0) {
        perror("Failed to open SPI bus");
        return -1;
    }

    if (ioctl(state->spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Failed to set SPI mode");
        close(state->spi_fd);
        state->spi_fd = -1;
        return -1;
    }
    if (ioctl(state->spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        perror("Failed to set SPI bits per word");
        close(state->spi_fd);
        state->spi_fd = -1;
        return -1;
    }
    if (ioctl(state->spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Failed to set SPI speed");
        close(state->spi_fd);
        state->spi_fd = -1;
        return -1;
    }

    *chip = gpiod_chip_open(GPIO_CHIP);
    if (!(*chip)) {
        perror("Failed to open GPIO chip");
        close(state->spi_fd);
        state->spi_fd = -1;
        return -1;
    }

    *line = gpiod_chip_get_line(*chip, GPIO_IRQ);
    if (!(*line)) {
        perror("Failed to get GPIO line");
        gpiod_chip_close(*chip);
        close(state->spi_fd);
        state->spi_fd = -1;
        return -1;
    }

    int ret = gpiod_line_request_rising_edge_events(*line, "as3935_irq");
    if (ret < 0) {
        perror("Failed to configure GPIO line");
        gpiod_line_release(*line);
        gpiod_chip_close(*chip);
        close(state->spi_fd);
        state->spi_fd = -1;
        return -1;
    }
    usleep(DELAY_200MS);
    return 0;
}

int handle_interrupt(struct SystemState *state, struct gpiod_line *line, struct EventCounters *counters) {
    int value = gpiod_line_get_value(line);
    if (value < 0) {
        perror("Failed to read GPIO value");
        return -1;
    }

    if (value == 1) {
        char buffer[20];
        log_timestamp(buffer, sizeof(buffer));

        usleep(DELAY_5MS); 

        uint8_t event, event_verify;
        if (spi_read_register(state, CONFIG_REG_3, &event) < 0) return -1;
        event &= 0x0F;

        int timeout_ms = 200;
        while (gpiod_line_get_value(line) == 1 && timeout_ms-- > 0) {
            usleep(DELAY_1MS);
        }
        if (timeout_ms <= 0) {
            fprintf(state->log_file, "%s - Timeout waiting for interrupt to clear\n", buffer);
            fflush(state->log_file);
            return -1;
        }

        switch (event) { 
            case 0x01:
                printf("Noise level too high ⚠️ (INT_NH) - Time: %s\n", buffer);
                fprintf(state->log_file, "%s - High noise level (INT_NH)\n", buffer);
                break;
            case 0x04:
                counters->noise_count++;
                printf("Interference detected 🌩 (INT_D). Event %d - Time: %s\n", counters->noise_count, buffer);
                fprintf(state->log_file, "%s - Interference (INT_D), Event %d\n", buffer, counters->noise_count);
                usleep(DELAY_1s5);
                break;
            case 0x08:
                counters->lightning_count++;
                uint8_t raw_distance;
                if (spi_read_register(state, CONFIG_REG_7, &raw_distance) < 0) return -1;
                raw_distance &= AS3935_REG_MASK;
                
                fprintf(state->log_file, "%s - Lightning detected (INT_L), Lightning %d, Distance: %d km\n", buffer, counters->lightning_count, raw_distance == 0x3F ? -1 : raw_distance);
                printf("¡Lightning %i detected! ⚡ (INT_L) - Time: %s\n", counters->lightning_count, buffer);

                if (raw_distance == 0x3F) { 
                    printf("Distance: Out of range (>40 km)\n");
                } else {
                    printf("Estimated distance: %d km\n", raw_distance);
                }
                
                usleep(DELAY_1s);
                break;
            default:
                counters->noise_count++;
                printf("Unknown event %d (0x%02X)\n", counters->noise_count, event);
                fprintf(state->log_file, "%s - Unknown event (0x%02X), Event %d\n", buffer, event, counters->noise_count);
        }
        fflush(state->log_file);
    }
    return 0;
}