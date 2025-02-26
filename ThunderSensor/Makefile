NAME ?= ThunderSensor
# Directorios
SRC_DIR = src
INC_DIR = include
THUNDER_DIR = Thunder_click
BUILD_DIR = build

# Archivos fuente
SRC_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(THUNDER_DIR)/*.c)

# Archivos objeto
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Compilador cruzado (ajustado según tu toolchain)
CC = /opt/cross-pi-gcc.14.2/bin/aarch64-none-linux-gnu-gcc
CFLAGS = -g -I$(INC_DIR) -I$(THUNDER_DIR) #-g es para poder depurar

# Ejecutable
EXEC = $(BUILD_DIR)/$(NAME) #Puedo poner el nombre de una variable (make name=)

# Regla por defecto: compilar todo
all: $(BUILD_DIR) $(EXEC)

# Regla para compilar el ejecutable
$(EXEC): $(OBJ_FILES)
	$(CC) -o $@ $^

# Regla para compilar archivos .c a .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para crear el directorio build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Limpiar archivos generados
clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean
