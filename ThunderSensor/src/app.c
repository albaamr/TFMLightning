/**
 * @file app.c
 * @brief Lightning detection application logic.
 *
 * @author Alba Moreno Ramos
 * @date 
 */

#include "app.h"
#include "raspi.h"
#include "AS3935.h"
#include <stdio.h>
#include <gpiod.h>

int run_app() {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    struct gpiod_line_event event;
    int ret;

    systemInit(&chip,&line);
    applicationInit();

    printf("Esperando detección de rayos en GPIO %d...\n", GPIO_IRQ);

    while (1) {
        ret = gpiod_line_event_wait(line, NULL);
        if (ret < 0) {
            perror("Error en espera de evento GPIO");
            break;
        }
        if (gpiod_line_event_read(line, &event) < 0) {
            perror("Error al leer el evento GPIO");
            break;
        }
        handle_interrupt(line);
    } 

    if (log_file) {
        fprintf(log_file, "Fin de sesión: %s\n", ctime(&(time_t){time(NULL)}));
        fclose(log_file);
    }
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    close(spi_fd); 
    return EXIT_SUCCESS;
}