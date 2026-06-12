#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../../include/process/runner.h"
#include "../../include/core/parser.h"
#include "../../include/process/control.h"
#include "../../include/utils/path_resolver.h"

// en caso de pipeline: si se proporciona out_pid la función no bloquea esperando al hijo.
// si es NULL, se asume que es un comando estándar y espera su finalización.
int run_process(CommandStruct *command, int input_fd, int output_fd, pid_t *out_pid) {
    char* executable_path = resolve_path(command->command);
    int last_status = -1;

    if (executable_path != NULL) {
        pid_t pid = fork();

        if (pid < 0) {
            printf("ucvsh: Error crítico durante la creación del proceso.\n");
            last_status = -1;
        } else if (pid == 0) {
            signal(SIGINT, SIG_DFL); // restablece el comportamiento por defecto.
            
            if (input_fd != STDIN_FILENO) {
                if (dup2(input_fd, STDIN_FILENO) < 0) {
                    perror("ucvsh: error en dup2 (entrada)");
                    exit(EXIT_FAILURE);
                }
                close(input_fd);
            }

            // configurar redirección de salida (archivo > o pipe).
            if (command->output_file != NULL) {
                int fd_out = open(command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("ucvsh: error al abrir archivo de salida");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd_out, STDOUT_FILENO) < 0) {
                    perror("ucvsh: error en dup2 (redirección archivo)");
                    exit(EXIT_FAILURE);
                }
                close(fd_out);
            } else if (output_fd != STDOUT_FILENO) {
                if (dup2(output_fd, STDOUT_FILENO) < 0) {
                    perror("ucvsh: error en dup2 (salida pipe)");
                    exit(EXIT_FAILURE);
                }
                close(output_fd);
            }

            execv(executable_path, command->cmd_args);
            perror("ucvsh: error en execv");
            exit(EXIT_FAILURE);

        } else { // proceso padre.
            // exportar el PID al executor para sincronización posterior.
            if (out_pid != NULL) *out_pid = pid;

            if (command->background == 1) {
                // registro de proceso asíncrono.
                add_job(pid, command->command);
                printf("\n[Proceso %d] ejecutándose en segundo plano\n", pid);
                fflush(stdout);
                last_status = 0;
            } else if (out_pid == NULL) {
                // comando externo (no pipeline): el proceso padre espera.
                int status;
                waitpid(pid, &status, 0);
                
                if (WIFEXITED(status)) {
                    last_status = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    printf("\n");
                    last_status = 128 + WTERMSIG(status);
                } else last_status = -1;

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