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
