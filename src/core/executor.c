#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/core/executor.h"
#include "../../include/core/parser.h"
#include "../../include/process/runner.h"
#include "../../include/process/control.h"
#include "../../include/builtins/exit.h"
#include "../../include/builtins/cd.h"
#include "../../include/builtins/pwd.h"
#include "../../include/builtins/clear.h"

int execute_command_list(CommandStruct **cmd_list, int cmd_count) {
    int last_status = 0;
    
    for (int i = 0; i < cmd_count; i++) {
        CommandStruct *command = cmd_list[i];

        // lógica de saltos de ejecución condicional.
        if (i > 0) {
            OperatorType prev_op = cmd_list[i - 1]->next_op;
            if (prev_op == OP_AND && last_status != 0) continue;
            if (prev_op == OP_OR && last_status == 0) continue;
        }

        if (command->command != NULL) {
            if (strcmp(command->command, "exit") == 0) {
                builtin_exit();
            }
            if (strcmp(command->command, "jobs") == 0) {
                _jobs();
                last_status = 0;
                continue;
            }
            if (strcmp(command->command, "cd") == 0) {
                last_status = builtin_cd((*cmd_list)->cmd_args);
                continue;
            }
            if (strcmp(command->command, "pwd") == 0) {
                last_status = builtin_pwd((*cmd_list)->cmd_args);
                continue;
            }
            if (strcmp(command->command, "clear") == 0) {
                last_status = builtin_clear((*cmd_list)->cmd_args);
                continue;
            }
        }

        // delegar la ejecución al sistema de procesos
        last_status = run_process(command);
    }

    return last_status;
}