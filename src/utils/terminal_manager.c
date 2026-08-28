#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "../../include/utils/terminal_manager.h"
#include "../../include/utils/history_manager.h"

/*# Este módulo se encarga de operar directamente con la capa de control de la terminal del 
    sistema operativo. Utiliza <termios.h> como la interfáz del estándar de Unix para controlar
    el comportamiento de los puertos de comunicación asíncrona.

    termios es una estructura POSIX que guarda todos los parámetros de la terminal (velocidad,
    caracteres especiales, modos locales, entre otros). 
    
    la función 'tcsetattr' aplica la configuración. La bandera 'TCSAFLUSH' le indica al sistema
    esperar a que se imprima todo lo que esté pendiente, descarte lo que aún no haya sido leído
    y entonces aplique la configuración original.'tcgetattr' obtiene la configuración actual de 
    la terminal. El fd 'STDIN_FILENO' indica que se quiere la configuración del flujo de entrada 
    estándar.

    La estructura termios posee un campo denominado 'c_lflag' (local flags), que es un panel de 
    interruptores de bits. 'ECHO' imprime automáticamente en pantalla cada tecla que se presiona. 
    Al apagar esta bandera, se controla manualmente cuándo se dibuja cada letra. 'ICANON' le 
    indica al OS que guarde todo lo que se escribe en un búfer temporal hasta que se presione la 
    tecla enter. Al apagar esta bandera, se reciben las pulsaciones instatáneamente byte a byte. 
    
    Por otra parte, c_cc[VMIN] y c_cc[VTIME] son reglas para la lectura. VMIN = 1 indica el número 
    mínimo de teclas que deben ser presionadas antes de leer, VTIME indica un límite de tiempo.
    */

static struct termios orig_termios;

// restaura la terminal a su estado original (modo canónico)
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// activa el modo no canónico (raw mode).
void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    // garantiza que se restaure si la shell colapsa de manera inesperada.
    atexit(disable_raw_mode); 

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    
    // se lee byte a byte sin tiempo de espera.
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// captura la entrada del usuario a bajo nivel
// retorna 1 si se leyó un comando; 0 si se recibió EOF (Ctrl+D).
int read_input_raw(char *buffer, size_t size) { 
    size_t pos = 0; // posición actual dle cursor.
    size_t len = 0; // longitud total del texto tecleado.
    char c;

    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == '\n') {
            write(STDOUT_FILENO, "\n", 1);
            break;
        } else if (c == 4) { // cód. ASCII 4 = EOT (Ctrl+D)
            if (pos == 0) return 0;
        } else if (c == 127 || c == '\b') { // backspace
            if (pos > 0) {
                // se mueve a una dir. de memoria hacia la iquierda para sobreescribir
                memmove(&buffer[pos - 1], &buffer[pos], len - pos);
                pos--;
                len--;

                // se mueve el cursor visualmente a la izquierda.
                write(STDOUT_FILENO, "\033[1D", 4);
                // se redibuja el resto del texto a la derecha.
                write(STDOUT_FILENO, &buffer[pos], len - pos);
                // imprime un espacio al final para limpiar la última letra residual
                write(STDOUT_FILENO, " ", 1);
            
                // devuelve el cursor a su posición correcta (ANSI: Mover Izquierda)
                char seq[32];
                snprintf(seq, sizeof(seq), "\033[%zuD", len - pos + 1);
                write(STDOUT_FILENO, seq, strlen(seq));
            }
        } else if (c == 27) { // cód. ASCII 27 = ESC (inicio de secuencia de escape)
            char seq[3];
            // se leen los siguientes dos bytes de la secuencia.
            if (read(STDIN_FILENO, &seq[0], 1) == 0) continue;
            if (read(STDIN_FILENO, &seq[1], 1) == 0) continue;

            const char *hist_cmd = NULL;
            if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'A': // flecha arriba 
                        hist_cmd = get_history_previous();
                        break;
                    case 'B': // flecha abajo
                        hist_cmd = get_history_next();
                        break;
                    case 'C': 
                        if (pos < len) {
                            pos++;
                            write(STDOUT_FILENO, "\033[1C", 4); // orden ANSI: mover derecha
                        }
                        break;
                    case 'D': 
                        if (pos > 0) {
                            pos--;
                            write(STDOUT_FILENO, "\033[1D", 4); // orden ANSI: mover izquierda
                        }
                        break;
                }
            }

            if (hist_cmd != NULL) {
                // borrar visualmente la línea actual tecleada.

                if (pos > 0) {
                    char seq_clear[32];
                    // (*) mueve el cursor 'pos' veces a la izquierda (\033[%zuD)
                    // borra desde el cursor hasta el final de la línea (\033[K)
                    snprintf(seq_clear, sizeof(seq_clear), "\033[%zuD\033[K", pos);
                    write(STDOUT_FILENO, seq_clear, strlen(seq_clear));
                }

                // copiar el comando del historial al buffer actual
                strncpy(buffer, hist_cmd, size - 1);
                buffer[size - 1] = '\0';
                len = strlen(buffer);
                pos = len; // el cursor se pone al final del nuevo comando.

                // imprimir el nuevo comando en pantalla.
                write(STDOUT_FILENO, buffer, len);
            }       
        } else { // carácter de texto normal
            if (len < size - 1) {
                // se hace espacio empujando la memoria a la derecha.
                memmove(&buffer[pos + 1], &buffer[pos], len - pos);
                buffer[pos] = c;
                // se dibuja el nuevo carácter y todo el texto que fue empujado.
                write(STDOUT_FILENO, &buffer[pos], len - pos + 1);

                pos++;
                len++;

                if (len > pos) {
                    char seq[32];
                    snprintf(seq, sizeof(seq), "\033[%zuD", len - pos);
                    write(STDOUT_FILENO, seq, strlen(seq));
                }
            }
        }
    }
    buffer[len] = '\0';
    return 1;
}