#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../parser/ast.h"

int execute_ast(ast_node_t *root) {
    if (!root) {
        return -1;
    }
    if (root->type == COMMAND_NODE) {
        return execute_command(root->command);
    }
    return 0;
}

int execute_command(char **command) {
    if (!command || !command[0]) {
        return -1;
    }
    if (execvp(command[0], command) == -1) {
        perror("execvp");
        return -1;
    }
    return 0;
}
