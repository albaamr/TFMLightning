#ifndef AS3935_H
#define AS3935_H

#include <stdint.h>

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
