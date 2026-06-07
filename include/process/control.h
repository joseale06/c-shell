#ifndef CONTROL_H
#define CONTROL_H

#include <sys/types.h>

typedef enum {
    RUNNING,
    SUSPENDED,
    FINISHED
} JobState;

typedef struct Job {
    pid_t pid;
    JobState state;
    char *command;
    struct Job *next;
} Job;

extern Job *job_list_head;

void add_job(pid_t pid, const char *cmd);
void update_job_state(pid_t pid, JobState new_state);
void builtin_jobs();
void builtin_exit();

#endif