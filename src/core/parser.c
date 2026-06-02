#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/core/parser.h"

#define MAX_ARGS 64 // cantidad máxima de argumentos por comando.

/* Delimitadores: 
    espacio (' ')
    tabulación ('\t')
    salto de línea ('\n') 
    retorno de carro ('\r') (protección ante entrada con formato de Windows).

    ej. si el usuario ingresó "ls -l", input contiene:
    ['l', 's', ' ', '-', 'l', '\0'] (ya se ha eliminado el salto de linea)

    strtok leerá de izq. a der. el arreglo 'input', buscando cualquier caracter delimitador
    entonces, reemplazará los delimitadores -> ['l', 's', '\0', '-', 'l', '\0'], además,
    devuelve un puntero al inicio del array (var. token).
*/
static void split_arguments(char *input, CommandStruct *cmd) {
    int i = 0;
    char *token = strtok(input, " \t\r\n");

    while (token != NULL && i < MAX_ARGS - 1) {
        cmd->cmd_args[i] = token;
        i++;
        token = strtok(NULL, " \t\r\n"); // llamadas subsecuentes a strtok usan NULL (tiene memoria interna).
    }
    // el arreglo de argumentos debe terminar en NULL para que execvp funcione posteriormente.
    cmd->cmd_args[i] = NULL;
    cmd->num_args = i; 
}

// analiza el arreglo de argumentos ya procesado.
// Si el último caracter es '&', enciende la bandera y lo elimina de la lista. 
static void run_in_background(CommandStruct *cmd) {
    cmd->background = 0;

    if (cmd->num_args > 0) {
        // se accede al último argumento válido.
        char *last_arg = cmd->cmd_args[cmd->num_args - 1];
        
        if (strcmp(last_arg, "&") == 0) {
            cmd->background = 1;
            cmd->cmd_args[cmd->num_args - 1] = NULL;
            cmd->num_args--;
        }
    }
}

void print_command_debug(CommandStruct *cmd) {
    if (cmd == NULL) {
        printf("[DEBUG] Comando nulo o vacío.\n");
        return;
    }

    printf("[DEBUG] Resultado de parser.c\n");
    printf("Comando a ejecutar: [ %s ]\n", cmd->command);
    
    for (int j = 1; j < cmd->num_args; j++) {
        printf("  Argumento %d: [%s]\n", j, cmd->cmd_args[j]);
    }
    
    if (cmd->background) {
        printf("[!] El comando se ejecutará en segundo plano (asíncrono)\n");
    }
    
    printf("----------------------------------------\n\n");
}

void freeCommandStruct(CommandStruct *cmd) {
    if (cmd != NULL) {
        if (cmd->cmd_args != NULL)
            free(cmd->cmd_args);
    }
    free(cmd);
}

CommandStruct* parseInput(char *input) {
    CommandStruct *cmd = (CommandStruct*) malloc(sizeof(CommandStruct));
    if (cmd == NULL) return NULL;

    // reservación de memoria para el arreglo de argumentos 
    cmd->cmd_args = (char**) malloc(MAX_ARGS * sizeof(char*));
    if (cmd->cmd_args == NULL) {
        free(cmd);
        return NULL;
    }

    split_arguments(input, cmd);
    if (cmd->num_args == 0) {
        freeCommandStruct(cmd);
        return NULL;
    }

    run_in_background(cmd);
    if (cmd->num_args == 0) {
        freeCommandStruct(cmd);
        return NULL;
    }

    cmd->command = cmd->cmd_args[0];
    return cmd;
}

