#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../../include/process/runner.h"
#include "../../include/utils/path_resolver.h"

int run_process(CommandStruct *command) {
    char* executable_path = resolve_path(command->command);
    int last_status = 0;

    if (executable_path != NULL) {
        pid_t pid = fork();

        if (pid < 0) {
            printf("ucvsh: Error crítico durante la creación del proceso.\n");
            last_status = -1;
        } else if (pid == 0) {
            execv(executable_path, command->cmd_args);
            exit(EXIT_FAILURE);
        } else {
            if (command->background == 0) {
                int status;
                waitpid(pid, &status, 0);
                last_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            } else {
                last_status = 0; 
            }
        }
        free(executable_path);
    } else {
        printf("ucvsh: %s: no se encontró la orden\n", command->command);
        last_status = 127; // comando no encontrado.
    }

    return last_status;
}