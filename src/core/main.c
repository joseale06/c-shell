#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../include/core/parser.h"
#include "../../include/utils/path_resolver.h"

#define MAX_BUFFER 1024

int main() {
    char input[MAX_BUFFER];

    while (1) {
        printf("\033[1;33mucvsh >\033[0m ");
        fflush(stdout);

        if(fgets(input, MAX_BUFFER, stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;

        int cmd_count = 0;
        CommandStruct **cmd_list = parseInput(input, &cmd_count);
        
        if (cmd_list == NULL) continue;

        int last_status = 0; // Almacena el resultado del comando anterior

        // Ejecutar cada comando en la lista respetando la lógica && y ||
        for (int i = 0; i < cmd_count; i++) {
            CommandStruct *command = cmd_list[i];

            #ifdef DEBUG
                print_command_debug(command);
            #endif

            // Lógica de salto por operadores
            if (i > 0) {
                OperatorType prev_op = cmd_list[i-1]->next_op;
                if (prev_op == OP_AND && last_status != 0) continue;
                if (prev_op == OP_OR && last_status == 0) continue;
            }

            char* executable_path = resolve_path(command->command);
            if (executable_path != NULL) {
                pid_t pid = fork();

                if (pid < 0) {
                    printf("ucvsh: Error crítico durante la creación del proceso.\n");
                } else if (pid == 0) {
                    execv(executable_path, command->cmd_args);
                    exit(EXIT_FAILURE);
                } else {
                    if (command->background == 0) {
                        int status;
                        waitpid(pid, &status, 0);
                        // Capturar si falló o tuvo éxito real
                        last_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
                    } else {
                        // Procesos asíncronos no afectan el && o ||
                        last_status = 0; 
                    }
                }
                free(executable_path);
            } else {
                printf("ucvsh: %s: no se encontró la orden\n", command->command);
                last_status = 127; // Código estándar de "comando no encontrado"
            }
        }
        
        freeCommandList(cmd_list, cmd_count);
    }
    return 0;
}