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

void run_pipeline(CommandStruct *cmd1, CommandStruct *cmd2) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("ucvsh: error al crear pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        char *path1 = resolve_path(cmd1->command);
        if (path1 == NULL) {
            fprintf(stderr, "ucvsh: %s: no se encontró la orden\n", cmd1->command);
            exit(127);
        }
        execv(path1, cmd1->cmd_args);
        perror("ucvsh: error en execv cmd1");
        free(path1);
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        char *path2 = resolve_path(cmd2->command);
        if (path2 == NULL) {
            fprintf(stderr, "ucvsh: %s: no se encontró la orden\n", cmd2->command);
            exit(127);
        }
        execv(path2, cmd2->cmd_args);
        perror("ucvsh: error en execv cmd2");
        free(path2);
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}