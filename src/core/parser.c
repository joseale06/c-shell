#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/core/parser.h"
void freeCommandList(CommandStruct **cmd_list, int cmd_count) {
    for (int i = 0; i < cmd_count; i++) {
        free(cmd_list[i]->command);
        free(cmd_list[i]->cmd_args);
        free(cmd_list[i]);
    }
    free(cmd_list);
}

CommandStruct **parseInput(char *input, int *cmd_count) {
    *cmd_count = 0;
    CommandStruct **cmd_list = malloc(sizeof(CommandStruct *) * 10);
    
    char *input_copy = strdup(input);
    char *token = strtok(input_copy, " ");
    while (token != NULL) {
        CommandStruct *cmd = malloc(sizeof(CommandStruct));
        cmd->cmd_args = malloc(sizeof(char *) * 10);
        cmd->background = 0;
        cmd->next_op = OP_NONE;

        if (strcmp(token, "&") == 0) {
            cmd->background = 1;
            token = strtok(NULL, " "); 
            if (token == NULL) break;
        }

        cmd->command = strdup(token);
        cmd->cmd_args[0] = cmd->command;

        int arg_idx = 1;
        token = strtok(NULL, " ");
        while (token != NULL && strcmp(token, "&") != 0 && strcmp(token, "&&") != 0 && strcmp(token, "||") != 0) {
            cmd->cmd_args[arg_idx++] = strdup(token);
            token = strtok(NULL, " ");
        }
        cmd->cmd_args[arg_idx] = NULL;

        if (token != NULL) {
            if (strcmp(token, "&&") == 0) cmd->next_op = OP_AND;
            else if (strcmp(token, "||") == 0) cmd->next_op = OP_OR;
            else if (strcmp(token, "&") == 0) cmd->background = 1;
        }
        
        cmd_list[(*cmd_count)++] = cmd;
        token = strtok(NULL, " ");
    }
    
    free(input_copy);
    return cmd_list;
}