#include <stdio.h>
#include <fcntl.h> //Librería para abrir archivos
#include <unistd.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h> //Envío de comandos de control al dispositivo
#include <string.h> //NO USADO DE MOMENTO

#define SPI_BUS "/dev/spidev0.0" // Depende del pin de la raspi. Pin 24 -> dev0.0. Pin 26 -> dev0.1
#define AS3935_REG_MASK 0x3F      // Solo los 6 bits menos significativos son válidos para direcciones

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
        .speed_hz = 500000, //fClk entre micro-sensor. Maximo 2 MHz, pero 500 kHz opción segura
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

int main() {
    spi_fd = open(SPI_BUS, O_RDWR); //Abrimos bus en modo lectura/escritura
    if (spi_fd < 0) {
        perror("Error al abrir el bus SPI");
        return 1;
    }

    uint8_t gain = spi_read_register(0x00); // Leer registro AFE (Analog Front End) gain. Amlifica y demodula señal AC de la antena. Se cambia en función de si lo usamos en interior/exterior
    printf("Ganancia inicial: 0x%02X\n", gain);

    spi_write_register(0x00, 0x24); // Escribir nuevo valor de ganancia (ejemplo)
    gain = spi_read_register(0x00);
    printf("Nueva ganancia: 0x%02X\n", gain);

    close(spi_fd);
    return 0;
}
