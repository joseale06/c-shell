#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../include/process/runner.h"
#include "../../include/core/parser.h"
#include "../../include/process/control.h"
#include "../../include/utils/path_resolver.h" 

int run_process(CommandStruct *command) {
    if (command == NULL || command->command == NULL) {
        return -1;
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