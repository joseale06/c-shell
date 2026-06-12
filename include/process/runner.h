#ifndef RUNNER_H  
#define RUNNER_H
#include "../../include/core/parser.h"

int run_process(CommandStruct *command);
void run_pipeline_multiple(CommandStruct **cmds, int count);
#endif