/**
 * @file app.c
 * @brief Lightning detection application logic.
 *
 * @author Alba Moreno Ramos
 * @version 0.1
 * @date 21-05-2025
 */

#include "app.h"
#include "raspi.h"
#include "AS3935.h"
#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <unistd.h>

/**
 * @brief Cleans up system resources.
 * 
 * @param state Pointer to system state containing SPI file descriptor and log file.
 * @param chip GPIO chip to close.
 * @param line GPIO line to release.
 */
static void cleanup(struct SystemState *state, struct gpiod_chip *chip, struct gpiod_line *line) {
    if (state->log_file) {
        char buffer[20];
        log_timestamp(buffer, sizeof(buffer));
        fprintf(state->log_file, "Session end: %s\n", buffer);
        fclose(state->log_file);
        state->log_file = NULL;
    }
    if (line) gpiod_line_release(line);
    if (chip) gpiod_chip_close(chip);
    if (state->spi_fd >= 0) {
        close(state->spi_fd);
        state->spi_fd = -1;
    }
}

int run_lightning_detection(void) {
    struct SystemState state = { .spi_fd = -1, .log_file = NULL };
    struct gpiod_chip *chip = NULL;
    struct gpiod_line *line = NULL;
    struct EventCounters counters = { .noise_count = 0, .lightning_count = 0 };
    struct gpiod_line_event event;

    if (systemInit(&state, &chip, &line) < 0) {
        cleanup(&state, chip, line);
        return EXIT_FAILURE;
    }

    if (as3935_init(&state) < 0) {
        cleanup(&state, chip, line);
        return EXIT_FAILURE;
    }
    
    printf("Waiting for lightning detection on GPIO %d...\n", GPIO_IRQ);

    while (1) {
        int ret = gpiod_line_event_wait(line, NULL);
        if (ret < 0) {
            perror("Failed to wait for GPIO event");
            cleanup(&state, chip, line);
            return EXIT_FAILURE;
        }
        if (gpiod_line_event_read(line, &event) < 0) {
            perror("Failed to read GPIO event");
            cleanup(&state, chip, line);
            return EXIT_FAILURE;
        }
        if (handle_interrupt(&state, line, &counters) < 0) {
            cleanup(&state, chip, line);
            return EXIT_FAILURE;
        }
    } 

    cleanup(&state, chip, line);
    return EXIT_SUCCESS;
}