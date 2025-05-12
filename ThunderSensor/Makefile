NAME ?= ThunderSensor
# Directorios
SRC_DIR = src
INC_DIR = include
#THUNDER_DIR = Thunder_click
BUILD_DIR = build
LIB_DIR = lib
#SYSROOT = /home/albaamr/alba_raspi #Ubicación donde monte el sistema completo de la raspi

# Archivos fuente
#SRC_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(THUNDER_DIR)/*.c)
#SRC_FILES = $(wildcard $(SRC_DIR)/*.c) #Para compilar todos los archivos .c
SRC_FILES = $(SRC_DIR)/DeteccionRF.c

# Archivos objeto
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Compilador cruzado (ajustado según tu toolchain)
CC = /opt/cross-pi-gcc.14.2/bin/aarch64-none-linux-gnu-gcc
CFLAGS = -g -I$(INC_DIR)  #-g es para poder depurar.
#CFLAGS = -g --sysroot=$(SYSROOT) -I$(INC_DIR)  #-g es para poder depurar.
#LDFLAGS = --sysroot=$(SYSROOT) -lgpiod -lrt # Para usar libgpiod con sysroot
LDFLAGS = -L$(LIB_DIR) -lgpiod -lrt # Para usar libgpiod copiando las librerías en mi proyecto desde la raspi


# Ejecutable
EXEC = $(BUILD_DIR)/$(NAME) #Puedo poner el nombre de una variable (make name=)

# Regla por defecto: compilar todo
all: $(BUILD_DIR) $(EXEC)

# Regla para compilar el ejecutable
$(EXEC): $(OBJ_FILES)
	$(CC) -o $@ $^ $(LDFLAGS)
#$(CC) -o $@ $^ $(LDFLAGS)

# Regla para compilar archivos .c a .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para crear el directorio build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Limpiar archivos generados
clean:
	rm -f $(OBJ_FILES) $(EXEC)

.PHONY: all clean
