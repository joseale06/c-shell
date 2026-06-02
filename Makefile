CC = gcc
CFLAGS = -Wall -Wextra -std=gnu17 -I./include
# incluir -g para símbolos de depuración

TARGET = ucvsh
SRC_DIR = src
OBJ_DIR = bin

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# ejecutar como 'make DEBUG=1'
ifdef DEBUG
	CFLAGS += -DDEBUG
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) $^ -o $@

# convierte cada .c individual en un archivo .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

run: all
	@./$(TARGET)

.PHONY: clean run
clean:
	@echo "Limpiando archivos binarios y objetos..."
	rm -rf $(OBJ_DIR) $(TARGET)




