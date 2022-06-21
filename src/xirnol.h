#ifndef _XIRNOL_H_ 
#define _XIRNOL_H_ 

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "dbg.h"
#include "val.h"
#include "xirnol_parse.h"

typedef struct eval_env_s {
    val_t stack;
    val_t vars;
    val_t vars_val;
    val_t bufs;
    val_t bufs_free;
  int32_t bufs_list_len;
} eval_env_t;

val_t kneval(ast_t ast);
int varcmp(const void *a, const void *b);

void gccycle(eval_env_t *env);

#endif