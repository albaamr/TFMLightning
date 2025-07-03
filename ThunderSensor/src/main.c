/**
 * @file main.c
 * @brief Main entry point for the lightning detection system.
 *
 * @author Alba Moreno Ramos
 * @version 0.1
 * @date 21-05-2025
 */
#include "app.h"
#include "AS3935.h"
#include "raspi.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Main function of the system.
 * 
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments
 * @return int System exit code (EXIT_SUCCESS or EXIT_FAILURE).
 */
int main(int argc, char *argv[]) {

    if (argc > 1 && strcmp(argv[1], "--tune") == 0) {
        struct SystemState state = { .spi_fd = -1, .log_file = NULL };
        struct gpiod_chip *chip = NULL;
        struct gpiod_line *line = NULL;

        // Initialize system
        if (systemInit(&state, &chip, &line) < 0) {
            cleanup(&state, chip, line);
            return EXIT_FAILURE;
        }

        // Initialize AS3935
        if (as3935_init(&state) < 0) {
            cleanup(&state, chip, line);
            return EXIT_FAILURE;
        }

        // Example tuning with division factor 1 (divide by 16) and TUN_CAP = 0
        uint8_t division_factor = 3; // Adjust as needed (0-3)
        uint8_t tune_cap = 9;        // Start with 0 pF, adjust from 0 to 15
        if (as3935_tune_antenna(&state, division_factor, tune_cap) < 0) {
            cleanup(&state, chip, line);
            return EXIT_FAILURE;
        }

        // Keep program running to measure frequency with logic analyzer
        printf("Tuning mode active. Measure frequency with logic analyzer\n");
       
        cleanup(&state, chip, line);
        return EXIT_SUCCESS;
    }

    //Normal lightning detection mode
    return run_lightning_detection();
}
