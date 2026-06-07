#ifndef PARSER_H
#define PARSER_H
typedef enum { OP_NONE, OP_SEMICOLON, OP_AND, OP_OR } OperatorType;

typedef struct {
    char *command;
    char **cmd_args;
    int num_args;
    int background;
    OperatorType next_op; 
} CommandStruct;

CommandStruct** parseInput(char *input, int *cmd_count);
void freeCommandList(CommandStruct **cmd_list, int cmd_count);
void print_command_debug(CommandStruct *cmd);

#endif