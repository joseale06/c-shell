#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../include/core/parser.h"
#include "../../include/utils/path_resolver.h"

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
        CommandStruct *command = parseInput(input);
        if (command == NULL) continue;

        #ifdef DEBUG
            print_command_debug(command);
        #endif

        char* executable_path = resolve_path(command->command);
        if (executable_path != NULL) {
            pid_t pid = fork();

            if (pid < 0) {
                printf("ucvsh: Error crítico durante la creación del proceso.");
            } else if (pid == 0) {
                execv(executable_path, command->cmd_args);

                //si exec() tuvo éxito, el proceso cargó el binario y no ejecutará estas líneas:
                printf("ucvsh: Error crítico de ejecución.");
                exit(EXIT_FAILURE);
            } else {
                // segmento para el proceso padre.
                if (command->background == 0) {
                    int status;
                    waitpid(pid, &status, 0);
                } else {
                    //ejecución asíncrona;
                }
            }
        } else {
            printf("ucvsh: %s: no se encontró la orden\n", command->command);
        }
        free(executable_path);
        freeCommandStruct(command);
    }
    return 0;
}