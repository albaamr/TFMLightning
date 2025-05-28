#ifndef RASPI_H
#define RASPI_H

#include <gpiod.h>
#include "AS3935.h"

/**
 * @file raspi.h
 * @brief Header for raspi.c, provides Raspberry Pi initialization and interrupt handling.
 * @author Alba Moreno Ramos
 * @version 0.1
 * @date 21-05-2025
 */

#define GPIO_CHIP "/dev/gpiochip0"
#define GPIO_IRQ 17 /**< GPIO pin used for AS3935 interrupt */

/**
 * @brief Structure to track event counts.
 */
struct EventCounters {
    int noise_count;
    int lightning_count;
};

/**
 * @brief Initialises SPI system and GPIO for interrupts.
 * 
 * @param state Pointer to system state containing SPI file descriptor.
 * @param chip Pointer to GPIO chip.
 * @param line Pointer to the GPIO (IRQ) line.
 * @return 0 on success, -1 on failure.
 */
int systemInit(struct SystemState *state, struct gpiod_chip **chip, struct gpiod_line **line);

/**
 * @brief Handles the AS3935 sensor interrupt.
 * 
 * @param state Pointer to system state containing log file.
 * @param line GPIO line where the interrupt was received.
 * @param counters Structure to track event counts (noise and lightning).
 * @return 0 on success, -1 on failure.
 */
int handle_interrupt(struct SystemState *state, struct gpiod_line *line, struct EventCounters *counters);

/**
 * @brief Formats current timestamp into a buffer.
 * 
 * @param buffer Buffer to store the formatted timestamp.
 * @param size Size of the buffer.
 */
void log_timestamp(char *buffer, size_t size);

#endif // RASPI_H