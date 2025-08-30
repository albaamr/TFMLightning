#ifndef AS3935_H
#define AS3935_H

#include <stdint.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

/**
 * @file AS3935.h
 * @brief Header for AS3935.c, provides functions for AS3935 sensor communication.
 * @author Alba Moreno Ramos
 * @version 0.1
 * @date 21-05-2025
 */

#define SPI_BUS "/dev/spidev0.0"
#define SPI_SPEED 1000000
#define DELAY_2MS 2000
#define DELAY_1MS 1000
#define DELAY_200MS 200000
#define DELAY_300MS 300000

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
#define AS3935_REG_MASK 0x3F /**< AS3935 uses 6-bit register addresses (0x00 to 0x3F) */

enum AS3935_Registers {
    CONFIG_REG_0 = 0x00,
    CONFIG_REG_1 = 0x01,
    CONFIG_REG_2 = 0x02,
    CONFIG_REG_3 = 0x03,
    CONFIG_REG_4 = 0x04,
    CONFIG_REG_5 = 0x05,
    CONFIG_REG_6 = 0x06,
    CONFIG_REG_7 = 0x07,
    CONFIG_REG_8 = 0x08,
    CONFIG_REG_3A = 0x3A,
    CONFIG_REG_3B = 0x3B,
    REG_CALIB_RCO = 0x3D,
    REG_PRESET_DEFAULT = 0x3C
};

/**
 * @brief System state structure to encapsulate resources.
 */
struct SystemState {
    int spi_fd;
    FILE *log_file;
};

/**
 * @brief Reads a register from the AS3935 sensor via spi.
 * 
 * @param state Pointer to system state containing SPI file descriptor.
 * @param reg Address of the register to read.
 * @param value Pointer to store the read value.
 * @return 0 on success, -1 on failure.
 */
int spi_read_register(struct SystemState *state, uint8_t reg, uint8_t *value);

/**
 * @brief Writes a value to an AS3935 sensor register via spi.
 * 
 * @param state Pointer to system state containing SPI file descriptor.
 * @param reg Address of the register to write.
 * @param value Value to write.
 * @return 0 on success, -1 on failure.
 */
int spi_write_register(struct SystemState *state, uint8_t reg, uint8_t value);

/**
 * @brief Initialises the AS3935 sensor.
 * 
 * @param state Pointer to system state containing SPI file descriptor.
 * @return 0 on success, -1 on failure.
 */
int as3935_init(struct SystemState *state);

#endif // AS3935_H
