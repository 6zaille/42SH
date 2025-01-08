#ifndef EXEC_H
#define EXEC_H

#include "../parser/ast.h"

// Fonctions d'exécution
int execute_ast(ast_node_t *root);
int execute_command(char **command);

#endif /* !EXEC_H */
