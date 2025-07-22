NAME ?= EField

# Directorios
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
SYSROOT = /opt/sysroot-pi
#LIB_DIR = lib

# Archivos fuente
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Archivos objeto
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Compilador cruzado (ajustado según tu toolchain)
CC = /opt/cross-pi-gcc.14.2/bin/aarch64-none-linux-gnu-gcc
#CFLAGS = -g -I$(INC_DIR)
CFLAGS = -g --sysroot=$(SYSROOT) -I$(INC_DIR) -I$(SYSROOT)/usr/include -I$(SYSROOT)/usr/include/aarch64-linux-gnu
LDFLAGS = --sysroot=$(SYSROOT) -L$(SYSROOT)/usr/lib/aarch64-linux-gnu -lpaho-mqtt3c -lpthread
#LDFLAGS = -lpaho-mqtt3c
#CFLAGS = -g --sysroot=$(SYSROOT) -I$(INC_DIR)  #-g es para poder depurar.
#LDFLAGS = --sysroot=$(SYSROOT) -lgpiod -lrt # Para usar libgpiod con sysroot
#LDFLAGS = -L$(LIB_DIR) -lgpiod -lrt # Para usar libgpiod copiando las librerías en mi proyecto desde la raspi


# Ejecutable
EXEC = $(BUILD_DIR)/$(NAME)

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
