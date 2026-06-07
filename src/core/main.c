#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../include/core/parser.h"
#include "../../include/core/executor.h"

#define MAX_BUFFER 1024 // tamaño máximo del búfer de entrada.

int main() {
    char input[MAX_BUFFER];

    while (1) {
        printf("\033[1;33mucvsh >\033[0m ");
        fflush(stdout); // asegura que el prompt 'ucvsh >' se imprima inmediatamente.

        // captura la entrada de forma segura
        // Si fgets() retorna NULL, significa que hubo un error o se detectó EOF (Ctrl + D)
        if(fgets(input, MAX_BUFFER, stdin) == NULL) break;

        // limpieza del salto de linea en el comando ingresado por el usuario.
        // se busca la posición de '\n' y se reemplaza por el carácter nulo '\0'.
        input[strcspn(input, "\n")] = '\0';

        // si se presiona enter (línea vacía), se vuelve al inicio.
        if (strlen(input) == 0) continue;

        //
        int cmd_count = 0;
        CommandStruct **cmd_list = parseInput(input, &cmd_count);

        #ifdef DEBUG
            print_command_debug(cmd_list);
        #endif

        if (cmd_list != NULL) {
            execute_command_list(cmd_list, cmd_count);
            freeCommandList(cmd_list, cmd_count);
        }
    }
    return 0;
}