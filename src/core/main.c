#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../include/core/parser.h"
#include "../../include/core/executor.h"
#include "../../include/process/control.h"
#include "../../include/utils/history_manager.h"
#include "../../include/utils/terminal_manager.h"

#define MAX_BUFFER 1024 // tamaño máximo del búfer de entrada.

int main() {
    char input[MAX_BUFFER];
    init_signals();
    init_history();

    while (1) {
        cleanup_jobs();
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("\033[1;33mucvsh:%s >\033[0m ", cwd);
        } else {
            printf("\033[1;33mucvsh >\033[0m ");
        }
        fflush(stdout); // asegura que el prompt 'ucvsh >' se imprima inmediatamente.

        enable_raw_mode();
        int status = read_input_raw(input, sizeof(input));
        disable_raw_mode();

        if (status == 0) {
            printf("Saliendo de ucvsh...\n");
            break;
        }

        // si se presiona enter (línea vacía), se vuelve al inicio.
        if (strlen(input) == 0) continue;

        //
        add_to_history(input);
        int cmd_count = 0;
        CommandStruct **cmd_list = parseInput(input, &cmd_count);

        #ifdef DEBUG
            print_command_debug(cmd_list);
        #endif

        if (cmd_list == NULL) continue;

        execute_command_list(cmd_list, cmd_count);
        freeCommandList(cmd_list, cmd_count);
    }

    return 0;
}