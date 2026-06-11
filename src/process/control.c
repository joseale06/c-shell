#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/process/control.h"

Job *job_list_head = NULL;

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

void builtin_jobs() {
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

void builtin_exit() {
    printf("Cerrando ucvsh...\n");
    // lógica para terminar todos los procesos activos.
    exit(0);
}