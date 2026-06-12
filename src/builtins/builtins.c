#include <string.h>
#include "../../include/builtins/builtins.h"
#include "../../include/builtins/cd.h"
#include "../../include/builtins/pwd.h"
#include "../../include/builtins/clear.h"
#include "../../include/builtins/exit.h"

int try_execute_builtin(CommandStruct *cmd, int *last_status) {
    if (cmd == NULL || cmd->command == NULL) return 0;

    if (strcmp(cmd->command, "cd") == 0) {
        *last_status = builtin_cd(cmd->cmd_args);
        return 1;
    }
    if (strcmp(cmd->command, "pwd") == 0) {
        *last_status = builtin_pwd(cmd->cmd_args);
        return 1;
    }
    if (strcmp(cmd->command, "clear") == 0) {
        *last_status = builtin_clear(cmd->cmd_args);
        return 1;
    }
    if (strcmp(cmd->command, "exit") == 0) {
        builtin_exit(); 
        return 1;
    }
    
    return 0; 
}