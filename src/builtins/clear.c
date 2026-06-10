#include <unistd.h>
#include "../../include/builtins/clear.h"

int builtin_clear(char **args) {
    (void)args;

    // \033[H: mueve el cursor a la esquina superior izquierda (0,0)
    // \033[J: borra desde la posición actual hasta el final de la pantalla
    write(STDOUT_FILENO, "\033[H\033[J", 7);
    return 0;
}