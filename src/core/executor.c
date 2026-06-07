#include "../../include/core/executor.h"
#include "../../include/process/runner.h"

int execute_command_list(CommandStruct **cmd_list, int cmd_count) {
    int last_status = 0;
    
    for (int i = 0; i < cmd_count; i++) {
        CommandStruct *command = cmd_list[i];

        // lógica de saltos de ejecución condicional.
        if (i > 0) {
            OperatorType prev_op = cmd_list[i-1]->next_op;
            if (prev_op == OP_AND && last_status != 0) continue;
            if (prev_op == OP_OR && last_status == 0) continue;
        }

        // delegar la ejecución al sistema de procesos
        last_status = run_process(command);
    }

    return last_status;
}