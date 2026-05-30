CC = gcc
CFLAGS = -Wall -Wextra -std=gnu17 # incluir -g para símbolos de depuración
TARGET=ucvsh
OBJDIR = bin
OBJS= $(OBJDIR)/main.o


# ejecutar como 'make DEBUG=1'
ifdef DEBUG
	CFLAGS += -DDEBUG
endif

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)
	@echo "¡Compilación exitosa! Listo para ejecutar (./$(TARGET))"

$(OBJDIR)/main.o: main.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR): # crea el dirctorio si no existe.
	mkdir -p $(OBJDIR)

.PHONY: clean run
clean:
	@echo "Limpiando archivos binarios y objetos..."
	rm -f $(OBJDIR) $(TARGET)


