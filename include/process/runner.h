#ifndef RUNNER_H  
#define RUNNER_H
#include "../../include/core/parser.h"

int run_process(CommandStruct *command);
void run_pipeline(CommandStruct *cmd1, CommandStruct *cmd2);
#endif