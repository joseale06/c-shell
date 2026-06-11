#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/core/executor.h"
#include "../../include/core/parser.h"
#include "../../include/process/runner.h"
#include "../../include/process/control.h"

int execute_command_list(CommandStruct **cmd_list, int cmd_count) {
    int last_status = 0;

    for (int i = 0; i < cmd_count; i++) {
        if (i > 0 && cmd_list[i - 1]->next_op == OP_PIPE) {
            continue;
        }

        CommandStruct *command = cmd_list[i];
        if (i > 0) {
            OperatorType prev_op = cmd_list[i - 1]->next_op;
            if (prev_op == OP_AND && last_status != 0) {
                continue;
            }
            if (prev_op == OP_OR && last_status == 0) {
                continue;
            }
        }

        if (command->next_op == OP_PIPE && i + 1 < cmd_count) {
            run_pipeline(command, cmd_list[i + 1]);
        } else {
            last_status = run_process(command);
        }
        if (command->command != NULL) {
            if (strcmp(command->command, "exit") == 0) {
                return 0;
            }
            if (strcmp(command->command, "jobs") == 0) {
                continue;
            }
        }
    }
    return last_status;
}