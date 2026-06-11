#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../../include/process/runner.h"
#include "../../include/core/parser.h"
#include "../../include/process/control.h"
#include "../../include/utils/path_resolver.h"

int run_process(CommandStruct *command) {
    char* executable_path;
    int last_status;
    pid_t pid;
    int status;
    int fd_out;

    executable_path = resolve_path(command->command);
    last_status = -1;

    if (executable_path != NULL) {
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "ucvsh: Error crítico durante la creación del proceso.\n");
            last_status = -1;
        } else if (pid == 0) {
            if (command->output_file != NULL) {
                fd_out = open(command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("ucvsh: error al abrir archivo de salida");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd_out, STDOUT_FILENO) < 0) {
                    perror("ucvsh: error en dup2 (salida)");
                    exit(EXIT_FAILURE);
                }
                close(fd_out);
            }

            execv(executable_path, command->cmd_args);
            perror("ucvsh: error en execv");
            exit(EXIT_FAILURE);
        } else {
            if (command->background == 1) {
                add_job(pid, command->command);
                last_status = 0;
            } else {
                waitpid(pid, &status, 0);
                
                if (WIFEXITED(status)) {
                    last_status = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    printf("\n");
                    last_status = 128 + WTERMSIG(status);
                } else {
                    last_status = -1;
                }
            }
        }
    } else {
        fprintf(stderr, "ucvsh: comando no encontrado: %s\n", command->command);
        last_status = 127; 
    }
    return last_status;
}

void run_pipeline_multiple(CommandStruct **cmds, int count) {
    int num_pipes = count - 1;
    int i, j;
    int status;
    int *pipefds;
    pid_t pid;

    pipefds = malloc(sizeof(int) * 2 * num_pipes);
    if (!pipefds) {
        perror("ucvsh: error de asignación de memoria para el pipeline");
        return;
    }

    for (i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("ucvsh: error al inicializar pipe");
            free(pipefds);
            return;
        }
    }

    for (i = 0; i < count; i++) {
        pid = fork();
        
        if (pid < 0) {
            perror("ucvsh: error al realizar fork en pipeline");
            free(pipefds);
            return;
        }

if (pid == 0) {
            if (i > 0) {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) < 0) {
                    perror("ucvsh: error en dup2 (entrada)");
                    exit(EXIT_FAILURE);
                }
            }

            if (i < count - 1) {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) < 0) {
                    perror("ucvsh: error en dup2 (salida)");
                    exit(EXIT_FAILURE);
                }
            } else {
                if (cmds[i]->output_file != NULL) {
                    int fd_out = open(cmds[i]->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_out < 0) {
                        perror("ucvsh: error al abrir archivo de salida");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(fd_out, STDOUT_FILENO) < 0) {
                        perror("ucvsh: error en dup2 (redireccion)");
                        exit(EXIT_FAILURE);
                    }
                    close(fd_out);
                }
            }

            for (j = 0; j < 2 * num_pipes; j++) {
                close(pipefds[j]);
            }

            char* executable_path = resolve_path(cmds[i]->command);
            if (executable_path != NULL) {
                execv(executable_path, cmds[i]->cmd_args);
            }

            fprintf(stderr, "ucvsh: comando no encontrado: %s\n", cmds[i]->command);
            exit(EXIT_FAILURE);
        }
    }

    for (j = 0; j < 2 * num_pipes; j++) {
        close(pipefds[j]);
    }

    for (i = 0; i < count; i++) {
        wait(&status);
    }

    free(pipefds);
}