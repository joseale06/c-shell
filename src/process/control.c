#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../include/process/control.h"

Job *job_list_head = NULL;

// ## MANEJO DE LA INTERRUPCIÓN CTRL + C
static void handle_sigint(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n\033[1;33mucvsh >\033[0m ", 21);
}

void init_signals() {
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);      // no bloquear otras señales mientras esta corre.
    sa.sa_flags = SA_RESTART;      // evita que otras funciones fallen al ser interrumpidas.
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error en sigaction");
    }
}
// ---------------------------------

void add_job(pid_t pid, const char *cmd) {
    Job *new_job = (Job *)malloc(sizeof(Job));
    if (new_job == NULL) {
        perror("Error al asignar memoria para el job");
        return;
    }

    new_job->pid = pid;
    new_job->state = RUNNING;
    new_job->command = strdup(cmd);
    new_job->next = job_list_head;
    job_list_head = new_job;
}

void update_job_state(pid_t pid, JobState new_state) {
    Job *current = job_list_head;
    while (current != NULL) {
        if (current->pid == pid) {
            current->state = new_state;
            return;
        }
        current = current->next;
    }
}

const char* get_state_string(JobState state) {
    switch(state) {
        case RUNNING: return "En ejecución";
        case SUSPENDED: return "Suspendido";
        case FINISHED: return "Finalizado";
        default: return "Desconocido";
    }
}

void _jobs() {
    Job *current = job_list_head;
    if (current == NULL) {
        printf("No existen activos.\n");
        return;
    }

    printf("PID\tEstado\t\tComando\n");
    while (current != NULL) {
        printf("%d\t%s\t%s\n", current->pid, get_state_string(current->state), current->command);
        current = current->next;
    }
}

void cleanup_jobs() {
    Job *current = job_list_head;
    Job *prev = NULL;

    while (current != NULL) {
        int status;
        pid_t result = waitpid(current->pid, &status, WNOHANG);

        if (result > 0) {
            if (prev == NULL) {
                job_list_head = current->next;
                free(current->command);
                free(current);
                current = job_list_head;
            } else {
                prev->next = current->next;
                free(current->command);
                free(current);
                current = prev->next;
            }
        } else {
            prev = current;
            current = current->next;
        }
    }
}