#include <stdio.h> //IO estándar
#include <fcntl.h> //Librería para abrir archivos
#include <unistd.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h> //Envío de comandos de control al dispositivo
#include <gpiod.h>  // Necesario para la interrupción. Librería para manejo de gpios libgpiod

#include <time.h>
//Con comillas busca en el propio proyecto

// FUNCIONES, REGISTROS, CONFIG, INTERACCION SENSOR FICHERO APARTE .C aqui no deberia haber referencia a raspberry pi
// Meter gpiochip, gpioirq, en un struct
// Hacerlo en otra carpeta, mantener este código. Empezar con el main
// 3 ficheros: raspi, app y sensor
// hablar con fernando representación de datos. practica
// no es estrictamente necesario hacer pcb (15 mayo aprox para que llegue). PROTOTIPO DE SISTEMA DE DETECCIÓN DE RAYOS

//-----------DEFINES-----------
// GPIO 17 en la Raspberry Pi Zero 2 W
#define GPIO_CHIP "/dev/gpiochip0"
#define GPIO_IRQ  17 // Pin GPIO donde está conectado IRQ  

#define SPI_BUS "/dev/spidev0.0" // Depende del pin de la raspi. Pin 24 -> dev0.0. Pin 26 -> dev0.1
#define AS3935_REG_MASK 0x3F      // Solo los 6 bits menos significativos son válidos para direcciones
#define SPI_SPEED 1000000  // fClk entre micro-sensor. Maximo 2 MHz, nunca poner 500 kHz (frecuencia de la antena)


// Valores registros
#define DIRECT_COMMAND 0x96      // Comando directo
#define AFE_GAIN_INDOOR 0x24
#define AFE_GAIN_OUTDOOR 0x1C
#define NORMAL_MODE 0x00
#define POWERDOWN_MODE 0x01

//Watchdog threshold. Cuando más grande, más robusto contra el ruido
#define CONFIG_WDTH_0 0x00
#define CONFIG_WDTH_1 0x01
#define CONFIG_WDTH_2 0x02 //Default
#define CONFIG_WDTH_3 0x03
#define CONFIG_WDTH_4 0x04
#define CONFIG_WDTH_5 0x05
#define CONFIG_WDTH_6 0x06
#define CONFIG_WDTH_7 0x07
#define CONFIG_WDTH_8 0x08
#define CONFIG_WDTH_9 0x09
#define CONFIG_WDTH_10 0x0A

//Noise Floor Level threshold
#define CONFIG_NFLT_0 0x00
#define CONFIG_NFLT_1 0x10
#define CONFIG_NFLT_2 0x20 //Default
#define CONFIG_NFLT_3 0x30
#define CONFIG_NFLT_4 0x40
#define CONFIG_NFLT_5 0x50
#define CONFIG_NFLT_6 0x60
#define CONFIG_NFLT_7 0x70

//Spike Rejection. Robustez frente a perturbaciones
#define CONFIG_SREJ_0 0x00
#define CONFIG_SREJ_1 0x01
#define CONFIG_SREJ_2 0x02 //Default
#define CONFIG_SREJ_3 0x03
#define CONFIG_SREJ_4 0x04
#define CONFIG_SREJ_5 0x05
#define CONFIG_SREJ_6 0x06
#define CONFIG_SREJ_7 0x07

//Numero mínimo de rayos
#define CONFIG_MIN_LIGHT_0 0x00
#define CONFIG_MIN_LIGHT_1 0x01
#define CONFIG_MIN_LIGHT_2 0x02 //Default
#define CONFIG_MIN_LIGHT_3 0x03

// <REGISTROS>
#define CONFIG_REG_0 0x00
#define CONFIG_REG_1 0x01
#define CONFIG_REG_2 0x02
#define CONFIG_REG_3 0x03
#define CONFIG_REG_4 0x04
#define CONFIG_REG_5 0x05
#define CONFIG_REG_6 0x06
#define CONFIG_REG_7 0x07 
#define CONFIG_REG_8 0x08
#define CONFIG_REG_3A 0x3A
#define CONFIG_REG_3B 0x3B
#define REG_CALIB_RCO 0x3D      // Registro para calibrar automáticamente los osciladores internos
#define REG_PRESET_DEFAULT 0x3C      // Registro para resetear los registros


int spi_fd; //Descriptor de archivo spi
int n = 0;
int r = 0; //Contabilizo los rayos
FILE *log_file = NULL; // Puntero global para el archivo de log


// Función para leer un registro del AS3935
uint8_t spi_read_register(uint8_t reg) { //AS3935.h
    //1ºByte: Dirección del registro con la máscara aplicada. 0x40 configura los bits MODE en modo de lectura
    //2ºByte: En SPI se deben enviar datos vacíos para recibir respuesta
    uint8_t tx_buf[2] = { (reg & AS3935_REG_MASK) | 0x40, 0x00 };  
    uint8_t rx_buf[2] = {0, 0}; //Respuesta del sensor
    struct spi_ioc_transfer transfer = { //Estructura para transferencia SPI
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = 2,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
    };
    
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        perror("Error en comunicación SPI");
        return 0xFF; // Devuelve un valor inválido
    }
    
    return rx_buf[1];
}

// Función para escribir en un registro del AS3935
void spi_write_register(uint8_t reg, uint8_t value) { //AS3935.h
    //1ºByte: Dirección del registro con la máscara aplicada. 0x00 configura los bits MODE en modo de escritura/comando (VER DATASHEET)
    uint8_t tx_buf[2] = { (reg & AS3935_REG_MASK) | 0x00, value };
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buf,
        .len = 2,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
    };
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        perror("Error en comunicación SPI");
        return; // Devuelve un valor inválido
    }
}

void systemInit(struct gpiod_chip **chip, struct gpiod_line **line) //Los asigno como punteros a punteros
{    
    uint8_t mode = SPI_MODE_1;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED;

    //-----CONFIGURACIONES-----
    spi_fd = open(SPI_BUS, O_RDWR); //Abrimos bus SPI en modo lectura/escritura
    if (spi_fd < 0) {
        perror("Error al abrir el bus SPI");
        return;
    }

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) { //Datos muestreados en el flanco de bajada de SCLK (CPHA=1)
        perror("Error al configurar el modo SPI");
        return;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        perror("Error al configurar bits por palabra en SPI");
        return;
    }
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Error al configurar la velocidad SPI");
        return;
    }

    *chip = gpiod_chip_open(GPIO_CHIP); //Abrir chip para acceder a los pines
    if (!(*chip)) {
        perror("Error al abrir el chip GPIO");
        return;
    }

    *line = gpiod_chip_get_line(*chip, GPIO_IRQ); //Controla GPIO 17
    if (!(*line)) {
        perror("Error al obtener la línea GPIO");
        gpiod_chip_close(*chip);
        return;
    }

    int ret = gpiod_line_request_rising_edge_events(*line, "as3935_irq"); //Detecta interrupción cuando IRQ 0-->1
    if (ret < 0) {
        perror("Error al configurar la línea GPIO");
        gpiod_chip_close(*chip);
        close(spi_fd);
        return;
    }
    usleep(200000); // Espera 200ms (como en codigo de ejemplo)
}

void applicationInit() //AS3935.h
{
    log_file = fopen("rayos.log", "a");
    if (!log_file) {
        perror("Error al abrir el archivo de log");
    } else {
        fprintf(log_file, "Inicio de sesión: %s\n", ctime(&(time_t){time(NULL)}));
        fflush(log_file);
    }

    //-----Calibración y reset-----
    spi_write_register(REG_PRESET_DEFAULT, DIRECT_COMMAND); // Comando de reset
    usleep(2000); // Espera 2ms (datasheet pagina 36)
    spi_write_register(REG_CALIB_RCO, DIRECT_COMMAND); // Comando de calibración RCO
    usleep(2000);

    //Verificar calibración
    uint8_t calib_status = spi_read_register(CONFIG_REG_3A);
    if (calib_status & 0x80) {
        printf("Calibración RCO exitosa\n");
        if (log_file) {
            fprintf(log_file, "Calibración RCO exitosa\n");
            fflush(log_file);
        }
    } else {
        printf("Error en calibración RCO. Revisa la antena o el sensor.\n");
        if (log_file) {
            fprintf(log_file, "Error en calibración RCO\n");
            fflush(log_file);
        }
    }

    spi_write_register(CONFIG_REG_8, 0x00);

    printf("CONFIG_REG_1: 0x%02X\n", spi_read_register(CONFIG_REG_1)); //CREO QUE LO QUE SE HACE MAL ES LA LECTURA. ANALIZADOR LÓGICO

    //-----Configuraciones-----
    spi_write_register(CONFIG_REG_0, AFE_GAIN_OUTDOOR | NORMAL_MODE); // Modificación ganancia AFE y modo de operación
    //spi_write_register(CONFIG_REG_0, AFE_GAIN_OUTDOOR | NORMAL_MODE); 
    uint8_t gain = spi_read_register(CONFIG_REG_0);
    printf("Ganancia AFE: 0x%02X\n", gain);

    spi_write_register(CONFIG_REG_1, CONFIG_NFLT_4 | CONFIG_WDTH_5); //Watchdog threshold y NFL
    spi_write_register(CONFIG_REG_2, CONFIG_MIN_LIGHT_0 | CONFIG_SREJ_3); //Spike rejection

    spi_write_register(CONFIG_REG_3, spi_read_register(CONFIG_REG_3) | 0x20); //Habilida MASK_DIST para ignorar interferencias
    spi_write_register(CONFIG_REG_8, 0x00);

    //------Trazas de comprobaciones----------
    printf("CONFIG_REG_1: 0x%02X\n", spi_read_register(CONFIG_REG_1));
    printf("CONFIG_REG_2: 0x%02X\n", spi_read_register(CONFIG_REG_2));
    printf("CONFIG_REG_3: 0x%02X\n", spi_read_register(CONFIG_REG_3));
    printf("CONFIG_REG_8: 0x%02X\n", spi_read_register(CONFIG_REG_8));

    usleep(300000); // Espera 300ms (como en codigo de ejemplo)
    printf("Sistema inicializado\n");

}

// Función para leer la interrupción y clasificar el evento. Se ejecuta cada vez que se detecta interrupción en GPIO17
void handle_interrupt(struct gpiod_line *line) {
    int value = gpiod_line_get_value(line); //Obtiene estado de GPIO
    if (value < 0) {
        perror("Error al leer el valor del GPIO");
        return;
    }

    if (value == 1) {
        
        time_t t;
        struct tm *tm_info;
        char buffer[20];

        // Leer el registro 0x03 del AS3935 para saber qué tipo de evento ocurrió
        uint8_t event = spi_read_register(CONFIG_REG_3) & 0x0F;
        uint8_t PRUEBA = spi_read_register(CONFIG_REG_3) & 0x0F;
        do {
            usleep(1000); 
        } while (gpiod_line_get_value(line) == 1); //Compruebo que IRQ vuelve a 0 después de haber leído el registro de interrupción


        time(&t);
        tm_info = localtime(&t);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

        printf("⚡ Interrupción detectada: "); 
        if (log_file) {
            switch (event) { 
                case 0x01:
                    printf("Ruido demasiado alto ⚠️ (INT_NH) - Hora: %s\n", buffer);
                    fprintf(log_file, "%s - Ruido alto (INT_NH)\n", buffer);
                    break;
                case 0x04:
                    n++;
                    printf("Interferencia detectada 🌩 (INT_D). Evento %i - Hora: %s\n",n,buffer);
                    fprintf(log_file, "%s - Interferencia (INT_D), Evento %i\n", buffer, n);
                    break;
                case 0x08:
                    r++;

                    uint8_t distancia_raw = spi_read_register(0x07) & 0x3F;
                    fprintf(log_file, "%s - Rayo detectado (INT_L), Rayo %i, Distancia: %d km\n", buffer, r, distancia_raw == 0x3F ? -1 : distancia_raw);
                    printf("¡Rayo %i detectado! ⚡ (INT_L) - Hora: %s\n", r, buffer);

                    // Leer la distancia del rayo desde el registro 0x07
                    //uint8_t distancia_raw = spi_read_register(0x07) & 0x3F; // Solo los bits 5:0 son válidos

                    if (distancia_raw == 0x3F) { // 0x3F significa "fuera de rango"
                        printf("Distancia: Fuera de rango (>40 km)\n");
                    } else {
                        printf("Distancia estimada: %d km\n", distancia_raw);
                    }
                    
                    break;
                default:
                    n++;
                    printf("Evento %i desconocido (0x%02X)\n", n, event);
                    fprintf(log_file, "%s - Evento desconocido (0x%02X), Evento %i\n", buffer, event, n);
            }
            fflush(log_file);
        }
    }
}

////////////////////////////////////////

int main() {
    struct gpiod_chip *chip; //Controlador GPIO
    struct gpiod_line *line; //Representa GPIO 17
    struct gpiod_line_event event; //Info de interrupción
    int ret;

    systemInit(&chip,&line);
    applicationInit();

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

    if (log_file) {
        fprintf(log_file, "Fin de sesión: %s\n", ctime(&(time_t){time(NULL)}));
        fclose(log_file);
    }
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    close(spi_fd); 
    return EXIT_SUCCESS;
}
////////////////////////////