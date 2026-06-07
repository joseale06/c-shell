#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/core/parser.h"

#define MAX_ARGS 64

static void split_arguments(char *input, CommandStruct *cmd) {
    int i = 0;
    char *saveptr; // Usar strtok_r para ser thread-safe/reentrante
    char *token = strtok_r(input, " \t\r\n", &saveptr);

    while (token != NULL && i < MAX_ARGS - 1) {
        cmd->cmd_args[i] = token;
        i++;
        token = strtok_r(NULL, " \t\r\n", &saveptr);
    }
    cmd->cmd_args[i] = NULL;
    cmd->num_args = i; 
}

static void run_in_background(CommandStruct *cmd) {
    cmd->background = 0;
    if (cmd->num_args > 0) {
        char *last_arg = cmd->cmd_args[cmd->num_args - 1];
        if (strcmp(last_arg, "&") == 0) {
            cmd->background = 1;
            cmd->cmd_args[cmd->num_args - 1] = NULL;
            cmd->num_args--;
        }
    }
}

void print_command_debug(CommandStruct *cmd) {
    if (cmd == NULL) return;
    printf("[DEBUG] Comando: [ %s ] | Operador siguiente: %d\n", cmd->command, cmd->next_op);
    for (int j = 1; j < cmd->num_args; j++) {
        printf("  Arg %d: [%s]\n", j, cmd->cmd_args[j]);
    }
    if (cmd->background) printf("[!] Asíncrono (&)\n");
}

void freeCommandList(CommandStruct **cmd_list, int cmd_count) {
    if (cmd_list == NULL) return;
    for (int i = 0; i < cmd_count; i++) {
        if (cmd_list[i] != NULL) {
            free(cmd_list[i]->cmd_args);
            free(cmd_list[i]);
        }
    }
    free(cmd_list);
}

// NUEVA VERSIÓN: Retorna un arreglo de CommandStructs
CommandStruct** parseInput(char *input, int *cmd_count) {
    CommandStruct **list = malloc(20 * sizeof(CommandStruct*));
    *cmd_count = 0;
    
    char *ptr = input;
    while (*ptr != '\0') {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '\0') break;

        char *cmd_start = ptr;
        OperatorType next_op = OP_NONE;
        
        // Buscar operadores lógicos
        while (*ptr != '\0') {
            if (strncmp(ptr, "&&", 2) == 0) {
                next_op = OP_AND; *ptr = '\0'; ptr += 2; break;
            } else if (strncmp(ptr, "||", 2) == 0) {
                next_op = OP_OR; *ptr = '\0'; ptr += 2; break;
            } else if (*ptr == ';') {
                next_op = OP_SEMICOLON; *ptr = '\0'; ptr += 1; break;
            }
            ptr++;
        }
        
        CommandStruct *cmd = malloc(sizeof(CommandStruct));
        cmd->cmd_args = malloc(MAX_ARGS * sizeof(char*));
        cmd->next_op = next_op;
        
        // Usamos tus funciones de limpieza y formato
        split_arguments(cmd_start, cmd);
        
        if (cmd->num_args > 0) {
            run_in_background(cmd);
            cmd->command = cmd->cmd_args[0];
            list[*cmd_count] = cmd;
            (*cmd_count)++;
        } else {
            free(cmd->cmd_args);
            free(cmd);
        }
    }

    if (*cmd_count > 0) {
        list[*cmd_count - 1]->next_op = OP_NONE;
    } else {
        free(list);
        return NULL;
    }

    return list;
}