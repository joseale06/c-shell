#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../include/core/executor.h"
#include "../../include/process/runner.h"
#include "../../include/process/control.h" // únicamente para poder llamar a jobs.
#include "../../include/builtins/builtins.h"

int execute_command_list(CommandStruct **cmd_list, int cmd_count) {
    int last_status = 0;
    int input_fd = STDIN_FILENO; // para pipeline: inicialmente, se lee del teclado.
    int pipefds[2];

    pid_t pipeline_pids[100]; // búfer para guardar procesos concurrentes.
    int pipeline_count = 0;
    
    for (int i = 0; i < cmd_count; i++) {
        CommandStruct *command = cmd_list[i];

        // lógica de saltos de ejecución condicional.
        // esto se evalúa solo si no es está en medio de un pipeline activo.
        if (i > 0 && input_fd == STDIN_FILENO) {
            OperatorType prev_op = cmd_list[i - 1]->next_op;
            if (prev_op == OP_AND && last_status != 0) continue;
            if (prev_op == OP_OR && last_status == 0) continue;
        }

        int is_pipe = (command->next_op == OP_PIPE);
        int output_fd = STDOUT_FILENO; // inicialmente, se escribe a pantalla.

        // si hay un pipe hacia el próximo comando, se inicializa la tubería.
        if (is_pipe) {
            if (pipe(pipefds) < 0) {
                perror("ucvsh: error inicializando pipe");
                break;
            }
            output_fd = pipefds[1]; // redirigir la salida hacia la tubería.
        }

        if (command->command != NULL) {
            int builtin_status = 0;
            
            if (try_execute_builtin(command, &builtin_status)) {
                last_status = builtin_status;
            } else if (strcmp(command->command, "jobs") == 0) {
                _jobs();
                last_status = 0;
            } else {
                // delegar la ejecución al sistema de procesos externos.
                int is_in_pipeline = (input_fd != STDIN_FILENO || is_pipe);
                pid_t child_pid;

                if (is_in_pipeline) {
                    run_process(command, input_fd, output_fd, &child_pid);
                    pipeline_pids[pipeline_count++] = child_pid;
                } else {
                    last_status = run_process(command, STDIN_FILENO, STDOUT_FILENO, NULL);
                }
            }
        }
        // # GESTIÓN DE DESCRIPTORES DEL PADRE -------------------------------------------
        
        // cerrar el extremo de lectura anterior que el hijo actual ya consumió.
        if (input_fd != STDIN_FILENO) close(input_fd);
        
        if (is_pipe) {
            close(pipefds[1]);    
            input_fd = pipefds[0]; // guardar lectura para el siguiente comando.
        } else {
            input_fd = STDIN_FILENO; // resetear para la próxima instrucción lógica
            
            // si se venía de un pipeline y llegamos al final, esperamos a los procesos.
            if (pipeline_count > 0) {
                int status;
                for (int p = 0; p < pipeline_count; p++) {
                    waitpid(pipeline_pids[p], &status, 0);
                    
                    // el last_status de toda la tubería será el del último comando.
                    if (p == pipeline_count - 1) {
                        if (WIFEXITED(status)) {
                            last_status = WEXITSTATUS(status);
                        } else if (WIFSIGNALED(status)) {
                            last_status = 128 + WTERMSIG(status);
                        }
                    }
                }
                pipeline_count = 0; // reiniciar para el siguiente bloque.
            }
        }
    }

    return last_status;
}