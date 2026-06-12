#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/core/parser.h"

#define MAX_ARGS 5 // cantidad base de argumentos para un comando (escalable dinámicamente).
#define MAX_CMDS 3 // cantidad base de comandos encadenados esperados (escalable dinámicamente).

/* Delimitadores: 
    espacio (' ')
    tabulación ('\t')
    salto de línea ('\n') 
    retorno de carro ('\r') (protección ante entrada con formato de Windows).

    ej. si el usuario ingresó "ls -l", input contiene:
    ['l', 's', ' ', '-', 'l', '\0'] (ya se ha eliminado el salto de linea)

    -> ['l', 's', '\0', '-', 'l', '\0']
    
    Anteriormente se utilizaba strtok para evitar construir la lógica de iteración y evaluación 
    manual de la cadena. Finalmente, ésta fue necesaria para un control total de la gestión de la 
    cadena; existían dificultades a la hora de separar los argumentos.
*/

static void split_arguments(char *input, CommandStruct *cmd) {
    cmd->cmd_args = malloc(MAX_ARGS * sizeof(char*));
    if (!cmd->cmd_args) return;
    
    int i = 0;
    char *ptr = input;

    while (*ptr != '\0') {
        // se omiten espacios en blanco iniciales.
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n') ptr++;
        if (*ptr == '\0') break;

        // redimensionar el arreglo dinámicamente si hay demasiados argumentos.
        if (i >= MAX_ARGS - 1) {
            int new_capacity = MAX_ARGS * 2;
            char **temp = realloc(cmd->cmd_args, new_capacity * sizeof(char*));
            if (!temp) break;
            cmd->cmd_args = temp;
        }

        if (*ptr == '"' || *ptr == '\'') {
            char quote = *ptr;
            ptr++; // salta la comilla de apertura.
            cmd->cmd_args[i++] = ptr;
            
            // avanza hasta encontrar la comilla de cierre.
            while (*ptr != '\0' && *ptr != quote) ptr++;
            
            if (*ptr != '\0') {
                *ptr = '\0'; // reemplaza la comilla de cierre por el delimitador.
                ptr++;
            }

        } else {
            // lógica para comandos o flags (ej. -ls, /bin/)
            cmd->cmd_args[i++] = ptr;
            
            while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t' && *ptr != '\r' && *ptr != '\n') ptr++;
            
            if (*ptr != '\0') {
                *ptr = '\0'; // corta la palabra.
                ptr++;
            }
        }
    }
    cmd->cmd_args[i] = NULL;
    cmd->num_args = i;
}

static void extract_redirections(CommandStruct *cmd) {
    cmd->output_file = NULL;
    for (int i = 0; i < cmd->num_args; i++) {
        if (strcmp(cmd->cmd_args[i], ">") == 0) {
            if (i + 1 < cmd->num_args) {
                // capturar el archivo de salida.
                cmd->output_file = cmd->cmd_args[i + 1];
                
                // desplazar el resto de los argumentos para "borrar" el > y el archivo
                for (int j = i; j < cmd->num_args - 2; j++) {
                    cmd->cmd_args[j] = cmd->cmd_args[j + 2];
                }
                cmd->num_args -= 2;
                cmd->cmd_args[cmd->num_args] = NULL;
                i--;
            }
        }
    }
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

CommandStruct** parseInput(char *input, int *cmd_count) {
    CommandStruct **list = malloc(MAX_CMDS * sizeof(CommandStruct*));
    if (!list) return NULL;

    *cmd_count = 0;
    char *ptr = input;
    
    while (*ptr != '\0') {
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '\0') break;

        char *cmd_start = ptr;
        OperatorType next_op = OP_NONE;
        char current_quote = '\0'; // rastrea si estamos dentro de comillas.
        
        // se aisla el comando y se busca un posible operador.
        while (*ptr != '\0') {
            if (*ptr == '"' || *ptr == '\'') {
                if (current_quote == '\0') {
                    current_quote = *ptr;
                } else if (current_quote == *ptr) {
                    current_quote = '\0';
                }
            }

            // solo detectar operadores lógicos si NO estamos dentro de comillas.
            if (current_quote == '\0') {
                if (strncmp(ptr, "&&", 2) == 0) {
                    next_op = OP_AND; 
                    *ptr = '\0'; ptr += 2; 
                    break;
                } else if (strncmp(ptr, "||", 2) == 0) {
                    next_op = OP_OR; 
                    *ptr = '\0'; ptr += 2; 
                    break;
                } else if (*ptr == '|') {        
                    next_op = OP_PIPE; 
                    *ptr = '\0'; ptr += 1; 
                    break;
                } else if (*ptr == ';') {
                    next_op = OP_SEMICOLON; 
                    *ptr = '\0'; ptr += 1; 
                    break;
                }
            }
            ptr++;
        }
        
        // se verifica la capacidad del arreglo para evitar desbordamiento.
        if (*cmd_count >= MAX_CMDS - 1) {
            int new_capacity = MAX_CMDS * 2;
            CommandStruct **temp_list = realloc(list, new_capacity * sizeof(CommandStruct*));
            if (!temp_list) {
                freeCommandList(list, *cmd_count);
                return NULL;
            }
            list = temp_list;
        }

        CommandStruct *cmd = calloc(1, sizeof(CommandStruct));
        if (!cmd) continue;
        
        cmd->next_op = next_op;
        split_arguments(cmd_start, cmd);
        extract_redirections(cmd);
        
        // filtrado de comandos vacíos generados por exceso de operadores.
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
