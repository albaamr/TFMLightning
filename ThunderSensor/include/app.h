/**
 * @file app.h
 * @brief Header for app.c
 * @author Alba Moreno Ramos
 * @version 0.1
 * @date 21-05-2025
 */

#ifndef APP_H
#define APP_H

#include "AS3935.h"
#include <gpiod.h>
 
/**
 * @brief Main system logic.
 * @return int System exit code (EXIT_SUCCESS or EXIT_FAILURE).
 */
int run_lightning_detection(void);

/**
 * @brief Cleans up system resources.
 * 
 * @param state Pointer to system state containing SPI file descriptor and log file.
 * @param chip GPIO chip to close.
 * @param line GPIO line to release.
 */
void cleanup(struct SystemState *state, struct gpiod_chip *chip, struct gpiod_line *line);

#endif // APP_H