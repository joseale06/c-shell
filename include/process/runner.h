#ifndef RUNNER_H  
#define RUNNER_H
#include "../../include/core/parser.h"

int run_process(CommandStruct *command, int input_fd, int output_fd, pid_t *out_pid);

#endif