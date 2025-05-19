#ifndef RASPI_H
#define RASPI_H

#include <gpiod.h>
#define GPIO_IRQ 17

/**
 * @file raspi.h
 * @brief Header for raspi.c
 * @author Alba Moreno Ramos
 * @version 0.1
 * @date 
 */

/**
 * @brief Initialises SPI system and GPIO for interrupts.
 * 
 * @param chip Pointer to GPIO chip.
 * @param line Pointer to the GPIO (IRQ) line.
 */
void systemInit(struct gpiod_chip **chip, struct gpiod_line **line);

/**
 * @brief Handles the AS3935 sensor interrupt.
 * 
 * @param line GPIO line where the interrupt was received.
 */
void handle_interrupt(struct gpiod_line *line);

#endif // RASPI_H