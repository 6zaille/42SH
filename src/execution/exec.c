#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

int execute_ast(struct ast *root) {
    if (!root) {
        return -1;
    }
    if (root->type == AST_NUMBER) {
        printf("Executing AST node with value: %zd\n", root->value);
        return 0;
    }
    if (root->type == AST_PLUS || root->type == AST_MINUS ||
        root->type == AST_MUL || root->type == AST_DIV) {
        int left_status = execute_ast(root->left);
        int right_status = execute_ast(root->right);
        return left_status || right_status;
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