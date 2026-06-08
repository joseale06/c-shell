#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../include/process/runner.h"
#include "../../include/utils/path_resolver.h"
#include "../../include/process/control.h"

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
            if (command->background == 1) {
            add_job(pid, command->command);
            printf("[Lanzado al fondo] PID: %d -> %s\n", pid, command->command);
            return 0; 
            }
            else if (command->background == 0) {
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

    char *executable_path = resolve_path(command->command);
    
    if (executable_path == NULL) {
        printf("ucvsh: %s: no se encontró la orden\n", command->command);
        return 127;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("ucvsh: Error crítico al hacer fork");
        free(executable_path);
        return 1;
    } 
    if (pid == 0) {
        execv(executable_path, command->cmd_args);
        perror("ucvsh: error en execv");
        free(executable_path);
        exit(EXIT_FAILURE);
    } 
    
    else {
        free(executable_path); 

        int status = 0;

        if (command->background == 1) {
            add_job(pid, command->command);
            printf("[%d] %s\n", pid, command->command);
            return 0; 
        } else {

            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
            return 0;
        }
    }
}