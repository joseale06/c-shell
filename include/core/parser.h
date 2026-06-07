#ifndef PARSER_H
#define PARSER_H

// Tipos de operadores lógicos
typedef enum { OP_NONE, OP_SEMICOLON, OP_AND, OP_OR } OperatorType;

typedef struct {
    char *command;
    char **cmd_args;
    int num_args;
    int background;
    OperatorType next_op; // Necesario para saber qué operador sigue
} CommandStruct;

// Prototipos actualizados
CommandStruct** parseInput(char *input, int *cmd_count);
void freeCommandList(CommandStruct **cmd_list, int cmd_count);
void print_command_debug(CommandStruct *cmd);

#endif