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
    
    // Limpieza inicial del prompt
    if (token != NULL && strcmp(token, "ucvsh>") == 0) {
        token = strtok(NULL, " ");
    }

    while (token != NULL) {
        CommandStruct *cmd = malloc(sizeof(CommandStruct));
        cmd->cmd_args = malloc(sizeof(char *) * 10);
        cmd->background = 0;
        cmd->next_op = OP_NONE;
        cmd->command = NULL;
        cmd->output_file = NULL; 
        if (strcmp(token, "&") == 0) {
            cmd->background = 1;
            token = strtok(NULL, " ");
            if (token == NULL) {
                cmd_list[(*cmd_count)++] = cmd;
                break;
            }
        }
        if (strcmp(token, "&&") == 0) {
            cmd->next_op = OP_AND;
            token = strtok(NULL, " ");
        } else if (strcmp(token, "||") == 0) {
            cmd->next_op = OP_OR;
            token = strtok(NULL, " ");
        } else if (strcmp(token, "|") == 0) {
            cmd->next_op = OP_PIPE;
            token = strtok(NULL, " ");
        } else {
            cmd->command = strdup(token);
            cmd->cmd_args[0] = cmd->command;

            int arg_idx = 1;
            token = strtok(NULL, " ");
            
            while (token != NULL && 
                   strcmp(token, "&") != 0 && 
                   strcmp(token, "&&") != 0 && 
                   strcmp(token, "||") != 0 && 
                   strcmp(token, "|") != 0) {
                if (strcmp(token, ">") == 0) {
                    token = strtok(NULL, " "); 
                    if (token != NULL) {
                        cmd->output_file = strdup(token);
                    }
                    token = strtok(NULL, " ");
                    continue; 
                }

                cmd->cmd_args[arg_idx++] = strdup(token);
                token = strtok(NULL, " ");
            }
            cmd->cmd_args[arg_idx] = NULL;
        }

        cmd_list[(*cmd_count)++] = cmd;
        if (cmd->command == NULL && cmd->next_op == OP_NONE && cmd->background == 0) {
            break;
        }
    }

    free(input_copy);
    return cmd_list;
}