#include <stdio.h>
#include <stdlib.h>
#include "../../include/builtins/exit.h"

void builtin_exit() {
    printf("Cerrando ucvsh...\n");
    // lógica para terminar todos los procesos activos.
    exit(0);
}