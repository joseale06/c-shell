#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../../include/process/runner.h"
#include "../../include/core/parser.h"
#include "../../include/process/control.h"
#include "../../include/utils/path_resolver.h"

int run_process(CommandStruct *command) {
    char* executable_path = resolve_path(command->command);
    int last_status = -1;

    if (executable_path != NULL) {
        pid_t pid = fork();

        if (pid < 0) {
            printf("ucvsh: Error crítico durante la creación del proceso.\n");
            last_status = -1;
        } else if (pid == 0) {
            signal(SIGINT, SIG_DFL); // restablece el comportamiento por defecto.
            execv(executable_path, command->cmd_args);
            exit(EXIT_FAILURE);
        } else {
            if (command->background == 1) {
                // registro de proceso asíncrono.
                add_job(pid, command->command);
                last_status = 0;
            } else {
                int status;
                waitpid(pid, &status, 0);
                
                if (WIFEXITED(status)) {
                    last_status = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    printf("\n");
                    last_status = 128 + WTERMSIG(status);
                } else last_status = -1;
            }
        }
        free(executable_path);
    } else {
        printf("ucvsh: %s: no se encontró la orden\n", command->command);
        last_status = 127; // comando no encontrado.
    }

    return last_status;
}