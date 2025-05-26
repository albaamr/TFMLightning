/**
 * @file main.c
 * @brief Main entry point for the lightning detection system.
 *
 * @author Alba Moreno Ramos
 * @version 0.1
 * @date 21-05-2025
 */
#include "app.h"
#include <stdlib.h>

/**
 * @brief Main function of the system.
 * 
 * @param argc Number of command-line arguments (ignored).
 * @param argv Array of command-line arguments (ignored).
 * @return int System exit code (EXIT_SUCCESS or EXIT_FAILURE).
 */
int main(int argc, char *argv[]) {
    return run_lightning_detection();
}
