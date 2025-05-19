/**
 * @file AS3935.c
 * @brief Communication functions with the AS3935 sensor via spi.
 *
 * @author Alba Moreno Ramos
 * @date 
 */
 
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "AS3935.h"

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

#define REG_CALIB_RCO 0x3D
#define REG_PRESET_DEFAULT 0x3C

int spi_fd;
FILE *log_file = NULL;

uint8_t spi_read_register(uint8_t reg) { 
    uint8_t tx_buf[2] = { (reg & AS3935_REG_MASK) | AS3935_READ_MODE, 0x00 };  
    uint8_t rx_buf[2] = {0, 0}; 
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = 2,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
    };
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        perror("Error en comunicación SPI");
        return 0xFF;
    }
    
    return rx_buf[1];
}

void spi_write_register(uint8_t reg, uint8_t value) {
    uint8_t tx_buf[2] = { (reg & AS3935_REG_MASK) | AS3935_WRITE_MODE, value };
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buf,
        .len = 2,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
    };
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        perror("Error en comunicación SPI");
        return;
    }
}

void applicationInit()
{
    log_file = fopen("rayos.log", "a");
    if (!log_file) {
        perror("Error al abrir el archivo de log");
    } else {
        fprintf(log_file, "\n--------\nInicio de sesión: %s\n", ctime(&(time_t){time(NULL)}));
        fflush(log_file);
    }

    spi_write_register(REG_PRESET_DEFAULT, DIRECT_COMMAND);
    usleep(DELAY_2MS);
    spi_write_register(REG_CALIB_RCO, DIRECT_COMMAND);
    usleep(DELAY_2MS);

    uint8_t calib_TRCO_status = spi_read_register(CONFIG_REG_3A);
    uint8_t calib_SRCO_status = spi_read_register(CONFIG_REG_3B);
    if ((calib_TRCO_status & 0x80) && (calib_SRCO_status & 0x80)) {
        printf("Calibración RCO exitosa\n");
        if (log_file) {
            fprintf(log_file, "Calibración RCO exitosa\n");
            fflush(log_file);
        }
    } else {
        printf("Error en calibración RCO. Revisa la antena o el sensor.\n");
        if (log_file) {
            fprintf(log_file, "Error en calibración RCO\n");
            fflush(log_file);
        }
    }

    spi_write_register(CONFIG_REG_0, AFE_GAIN_OUTDOOR | NORMAL_MODE);
    spi_write_register(CONFIG_REG_1, CONFIG_NFLT_4 | CONFIG_WDTH_5);
    spi_write_register(CONFIG_REG_2, CONFIG_MIN_LIGHT_0 | CONFIG_SREJ_3);
    spi_write_register(CONFIG_REG_3, spi_read_register(CONFIG_REG_3) | 0x20);

    usleep(DELAY_300MS);
    printf("Sistema inicializado\n");

}