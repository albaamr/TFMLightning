#include <stdio.h> //IO estándar
#include <fcntl.h> //Librería para abrir archivos
#include <unistd.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h> //Envío de comandos de control al dispositivo
#include <gpiod.h>  // Necesario para la interrupción. Librería para manejo de gpios libgpiod
//Comillas busca en el propio proyecto

// GPIO 17 en la Raspberry Pi Zero 2 W
#define GPIO_CHIP "/dev/gpiochip0"
#define GPIO_IRQ  17 // Pin GPIO donde está conectado IRQ  

#define SPI_BUS "/dev/spidev0.0" // Depende del pin de la raspi. Pin 24 -> dev0.0. Pin 26 -> dev0.1
#define AS3935_REG_MASK 0x3F      // Solo los 6 bits menos significativos son válidos para direcciones
#define SPI_SPEED 500000  // fClk entre micro-sensor. Maximo 2 MHz, pero 500 kHz opción segura

int spi_fd; //Descriptor de archivo spi

// Función para leer un registro del AS3935
uint8_t spi_read_register(uint8_t reg) {
    //1ºByte: Dirección del registro con la máscara aplicada. 0x40 configura los bits MODE en modo de lectura
    //2ºByte: En SPI se deben enviar datos vacíos para recibir respuesta
    uint8_t tx_buf[2] = { (reg & AS3935_REG_MASK) | 0x40, 0x00 };  
    uint8_t rx_buf[2] = {0}; //Respuesta del sensor
    struct spi_ioc_transfer transfer = { //Estructura para transferencia SPI
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = 2,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
    };
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer);
    return rx_buf[1];
}

// Función para escribir en un registro del AS3935
void spi_write_register(uint8_t reg, uint8_t value) {
    //1ºByte: Dirección del registro con la máscara aplicada. 0x00 configura los bits MODE en modo de escritura/comando (VER DATASHEET)
    uint8_t tx_buf[2] = { (reg & AS3935_REG_MASK) | 0x00, value };
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buf,
        .len = 2,
        .speed_hz = 500000,
        .bits_per_word = 8,
    };
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer);
}

// Función para leer la interrupción y clasificar el evento. Se ejecuta cada vez que se detecta interrupción en GPIO17
void handle_interrupt(struct gpiod_line *line) {
    int value = gpiod_line_get_value(line); //Obtiene estado de GPIO
    if (value < 0) {
        perror("Error al leer el valor del GPIO");
        return;
    }

    if (value == 1) {
        // Leer el registro 0x03 del AS3935 para saber qué tipo de evento ocurrió
        uint8_t event = spi_read_register(0x03) & 0x0F;

        printf("⚡ Interrupción detectada: ");
        switch (event) { 
            case 0x01:
                printf("Ruido demasiado alto ⚠️ (INT_NH)\n");
                break;
            case 0x04:
                printf("Interferencia detectada 🌩 (INT_D)\n");
                break;
            case 0x08:
                printf("¡Rayo detectado! ⚡ (INT_L)\n");
                break;
            default:
                printf("Evento desconocido (0x%02X)\n", event);
        }
    }
}

int main() {
    struct gpiod_chip *chip; //Controlador GPIO
    struct gpiod_line *line; //Representa GPIO 17
    struct gpiod_line_event event; //Info de interrupción
    int ret;

    //-----CONFIGURACIONES-----
    spi_fd = open(SPI_BUS, O_RDWR); //Abrimos bus SPI en modo lectura/escritura
    if (spi_fd < 0) {
        perror("Error al abrir el bus SPI");
        return 1;
    }

    chip = gpiod_chip_open(GPIO_CHIP); //Abrir chip para acceder a los pines
    if (!chip) {
        perror("Error al abrir el chip GPIO");
        return EXIT_FAILURE;
    }

    line = gpiod_chip_get_line(chip, GPIO_IRQ); //Controla GPIO 17
    if (!line) {
        perror("Error al obtener la línea GPIO");
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    ret = gpiod_line_request_rising_edge_events(line, "as3935_irq"); //Detecta interrupción cuando IRQ 0-->1
    if (ret < 0) {
        perror("Error al configurar la línea GPIO");
        gpiod_chip_close(chip);
        close(spi_fd);
        return EXIT_FAILURE;
    }

    //-----MODIFICACIÓN REGISTROS-----
    spi_write_register(0x00, 0x24); // Modificación ganancia AFE. Valor de ganancia 0x24 indoor
    uint8_t gain = spi_read_register(0x00);
    printf("Ganancia AFE: 0x%02X\n", gain);
    spi_write_register(0x03, spi_read_register(0x03) | 0x20); //Habilida MASK_DIST para ignorar interferencias
    uint8_t maskdist = spi_read_register(0x03);
    printf("Registro 0x03: 0x%02X\n", maskdist);

    //-----COMIENZO DETECCIÓN-----
    printf("Esperando detección de rayos en GPIO %d...\n", GPIO_IRQ);

    while (1) {
        ret = gpiod_line_event_wait(line, NULL); //Bloquea ejecución hasta que ocurre la interrupción. No consume CPU
        if (ret < 0) {
            perror("Error en espera de evento GPIO");
            break;
        }
        if (gpiod_line_event_read(line, &event) < 0) { //Lee evento del GPIO
            perror("Error al leer el evento GPIO");
            break;
        }
        handle_interrupt(line); //Analiza la interrupción
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    close(spi_fd);
    return EXIT_SUCCESS;
}
