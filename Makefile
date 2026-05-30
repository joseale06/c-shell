CC = gcc
CFLAGS = -Wall -Wextra -std=gnu17 # incluir -g para símbolos de depuración
TARGET=ucvsh
OBJS=

all: $(TARGET)

$(TARGET) : $(OBJS)
	@echo "Preparando y enlazando el ejecutable $(TARGET)..."
	$(CC) $(OBJS)-o $(TARGET)
	@echo "¡Compilación exitosa! Listo para ejecutar (./$(TARGET))"

clean:
	@echo "Limpiando archivos binarios y objetos..."
	rm -f $(OBJS) $(TARGET)

.PHONY all clean