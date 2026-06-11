#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/core/executor.h"
#include "../../include/core/parser.h"
#include "../../include/process/runner.h"
#include "../../include/process/control.h"

int execute_command_list(CommandStruct **cmd_list, int cmd_count) {
    int last_status = 0;
    
    for (int i = 0; i < cmd_count; i++) {
        CommandStruct *command = cmd_list[i];
        if (i > 0) {
            OperatorType prev_op = cmd_list[i - 1]->next_op;
            if (prev_op == OP_AND && last_status != 0) continue;
            if (prev_op == OP_OR && last_status == 0) continue;
        }

        if (command->command != NULL) {
            if (strcmp(command->command, "exit") == 0) {
                return 0;
            }
    
            if (strcmp(command->command, "jobs") == 0) {
                continue;
            }
        }

        if (command->next_op == OP_PIPE) {
            int num_pipe_cmds = 1;
            while (i + num_pipe_cmds < cmd_count && cmd_list[i + num_pipe_cmds - 1]->next_op == OP_PIPE) {
                num_pipe_cmds++;
            }
            
            run_pipeline_multiple(&cmd_list[i], num_pipe_cmds);
            i += num_pipe_cmds - 1; 
            continue;
        }
        last_status = run_process(command);
    }
    return last_status;
}
