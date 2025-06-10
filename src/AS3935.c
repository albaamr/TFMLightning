/**
 * @file AS3935.c
 * @brief Communication functions with the AS3935 sensor via spi.
 *
 * @author Alba Moreno Ramos
 * @date 21-05-2025
 */
 
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "AS3935.h"
#include "log.h"

#define NORMAL_MODE 0x00
#define POWERDOWN_MODE 0x01

#define AS3935_READ_MODE 0x40
#define AS3935_WRITE_MODE 0x00

#define DIRECT_COMMAND 0x96
#define AFE_GAIN_INDOOR 0x24
#define AFE_GAIN_OUTDOOR 0x1C

#define CONFIG_WDTH_0 0x00
#define CONFIG_WDTH_1 0x01
#define CONFIG_WDTH_2 0x02 //Default
#define CONFIG_WDTH_3 0x03
#define CONFIG_WDTH_4 0x04
#define CONFIG_WDTH_5 0x05
#define CONFIG_WDTH_6 0x06
#define CONFIG_WDTH_7 0x07
#define CONFIG_WDTH_8 0x08
#define CONFIG_WDTH_9 0x09
#define CONFIG_WDTH_10 0x0A

#define CONFIG_NFLT_0 0x00
#define CONFIG_NFLT_1 0x10
#define CONFIG_NFLT_2 0x20 //Default
#define CONFIG_NFLT_3 0x30
#define CONFIG_NFLT_4 0x40
#define CONFIG_NFLT_5 0x50
#define CONFIG_NFLT_6 0x60
#define CONFIG_NFLT_7 0x70

#define CONFIG_SREJ_0 0x00
#define CONFIG_SREJ_1 0x01
#define CONFIG_SREJ_2 0x02 //Default
#define CONFIG_SREJ_3 0x03
#define CONFIG_SREJ_4 0x04
#define CONFIG_SREJ_5 0x05
#define CONFIG_SREJ_6 0x06
#define CONFIG_SREJ_7 0x07

#define CONFIG_MIN_LIGHT_0 0x00
#define CONFIG_MIN_LIGHT_1 0x01
#define CONFIG_MIN_LIGHT_2 0x02 //Default
#define CONFIG_MIN_LIGHT_3 0x03

int __attribute__((weak)) spi_read_register(struct SystemState *state, uint8_t reg, uint8_t *value) { 
    uint8_t tx_buf[2] = { (reg & AS3935_REG_MASK) | AS3935_READ_MODE, 0x00 };  
    uint8_t rx_buf[2] = {0, 0}; 
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = 2,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
    };
    
    if (ioctl(state->spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        perror("Failed to read SPI register");
        return -1;
    }
    
    *value = rx_buf[1];
    return 0;
}

int __attribute__((weak)) spi_write_register(struct SystemState *state, uint8_t reg, uint8_t value) {
    uint8_t tx_buf[2] = { (reg & AS3935_REG_MASK) | AS3935_WRITE_MODE, value };
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buf,
        .len = 2,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
    };
    if (ioctl(state->spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        perror("Failed to write SPI register");
        return -1;
    }

    return 0;
}

int as3935_init(struct SystemState *state)
{
    if (state->log_file == NULL) {
        state->log_file = fopen("rayos.log", "a");
        if (!state->log_file) {
            perror("Failed to open log file");
            return -1;
        }
    }

    char buffer[20];
    log_timestamp(buffer, sizeof(buffer));
    fprintf(state->log_file, "\n--------\nSession start: %s\n", buffer);
    fflush(state->log_file);

    if (spi_write_register(state, REG_PRESET_DEFAULT, DIRECT_COMMAND) < 0) return -1;
    usleep(DELAY_2MS);
    if (spi_write_register(state, REG_CALIB_RCO, DIRECT_COMMAND) < 0) return -1;
    usleep(DELAY_2MS);

    uint8_t calib_TRCO_status, calib_SRCO_status;
    if (spi_read_register(state, CONFIG_REG_3A, &calib_TRCO_status) < 0 ||
        spi_read_register(state, CONFIG_REG_3B, &calib_SRCO_status) < 0) {
        return -1;
    }

    if ((calib_TRCO_status & 0x80) && (calib_SRCO_status & 0x80)) {
        printf("RC0 calibration successful\n");
        fprintf(state->log_file, "RC0 calibration successful\n");
        fflush(state->log_file);
    } else {
        printf("RC0 calibration failed. Check antenna or sensor.\n");
        fprintf(state->log_file, "RC0 calibration failed\n");
        fflush(state->log_file);
        return -1;
    }

    // Configure sensor for outdoor operation with specific noise and watchdog settings
    if (spi_write_register(state, CONFIG_REG_0, AFE_GAIN_INDOOR | NORMAL_MODE) < 0 ||
        spi_write_register(state, CONFIG_REG_1, CONFIG_NFLT_1 | CONFIG_WDTH_2) < 0 ||
        spi_write_register(state, CONFIG_REG_2, CONFIG_MIN_LIGHT_0 | CONFIG_SREJ_3) < 0) {
        return -1;
    }

    uint8_t reg3_value;
    if (spi_read_register(state, CONFIG_REG_3, &reg3_value) < 0 ||
        spi_write_register(state, CONFIG_REG_3, reg3_value | 0x20) < 0) {
        return -1;
    }

    usleep(DELAY_300MS);
    printf("System initialized\n");

}