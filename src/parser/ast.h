#ifndef AST_H
#define AST_H

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../execution/builtins.h"
#include "../execution/exec.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "ast.h"
#include "parser.h"

void ast_eval(struct ast *node);
void ast_pretty_print(struct ast *node, int depth);

#endif /* !AST_H */
