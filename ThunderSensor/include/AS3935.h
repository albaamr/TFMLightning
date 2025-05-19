#ifndef AS3935_H
#define AS3935_H

#include <stdint.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
extern int spi_fd;
extern FILE *log_file;

#define SPI_BUS "/dev/spidev0.0"
#define SPI_SPEED 1000000
#define DELAY_2MS 2000
#define DELAY_1MS 1000
#define DELAY_200MS 200000
#define DELAY_300MS 300000

#define AS3935_REG_MASK 0x3F

#define CONFIG_REG_0 0x00
#define CONFIG_REG_1 0x01
#define CONFIG_REG_2 0x02
#define CONFIG_REG_3 0x03
#define CONFIG_REG_4 0x04
#define CONFIG_REG_5 0x05
#define CONFIG_REG_6 0x06
#define CONFIG_REG_7 0x07 
#define CONFIG_REG_8 0x08
#define CONFIG_REG_3A 0x3A
#define CONFIG_REG_3B 0x3B

/**
 * @file AS3935.h
 * @brief Header for AS3935.c
 * @author Alba Moreno Ramos
 * @version 0.1
 * @date 
 */

/**
 * @brief Reads a register from the AS3935 sensor via spi.
 * 
 * @param reg Address of the register to read.
 * @return Value read from the register if success; 0xFF if fails.
 */
uint8_t spi_read_register(uint8_t reg);

/**
 * @brief Writes a value to an AS3935 sensor register via spi.
 * 
 * @param reg Address of the register to write.
 * @param value Value to write.
 */
void spi_write_register(uint8_t reg, uint8_t value);

/**
 * @brief Initialises the AS3935 sensor.
 */
void applicationInit(void);

#endif // AS3935_H
