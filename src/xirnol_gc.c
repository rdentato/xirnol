#include "xirnol.h"

/*
A very simple mark&sweep GC
Buffers and stacks need to be collected. We have a list of 
buffers in env.bufs and the list of stacks in env.stks

The "alive" objcets are those referenced in the evaluation stack,
in one of the variables or in on of the alive stackss. 

So we:

   - scan the stack and mark any buffers (valisbuf(x) == TRUE) or stack (valisvec(x) == TRUE)
   - if it's a stack, scan it as well.
   - scan the variables and do the same
   - scan the buf list and the stacks, move to the free list the ones that are not marked
     and unmark the others.

Note that some of the buffers are the name of dynamically created variables and those
must NOT be collected. They are marked with -1 to allow distinguishing them.

A GC cycle will be started when the number of allocated buffers is greater
(by a factor K) than the number of variables plus the depth of the stack.

*/
#define MIN_GC_CYCLE   20

//  50 25 12.5
//   1  1  1  1  1 1 1 1
#define GC_PROBABILITY  0xC0

static void sweep(eval_env_t *env)
{
 _dbgtrc("Move buffers to the free list");
  val_t  child;

  val_t *parent;
  int32_t *list_len;
  val_t *freelist;

    parent = &env->bufs;
  list_len = &env->bufs_list_len;
  freelist = &env->bufs_free;

  for (int k=0; k<2; k++) {
   _dbgtrc("SWEEP pass: %d, len: %d",k,*list_len);
    while ((child = *parent) != valnil) {
     _dbgmst(valisbuf(child) || valisvec(child));
     _dbgtrc("GC REFS: %d",valrefs(child));
      if (valrefs(child) != 0) { // MARKED
        // Unmark but not if it's the name of dynamically created variable (refs=-1)!
        if (valrefs(child) > 0) valrefs(child,0);  
        parent = valtoptr(child); //Only works because aux is the first field!
      }
      else { // Move to free list
        *parent = valnext(child); // Remove from list
        *list_len -= 1;
        valnext(child,*freelist); // Add to the free list
        valcount(child,0); // 
        *freelist = child;
      }
    }

      parent = &env->stks;
    list_len = &env->stks_list_len;
    freelist = &env->stks_free;
  }
}

static void mark(eval_env_t *env)
{
  val_t *arr ;
  val_t tocheck; // stack of vectors to check

  int32_t n;
  n=16+2*(env->stks_list_len);
  tocheck = valvec(n);

 _dbgtrc("GC ROOT: vars:%d (%d) stack: %d",valcount(env->vars_val),valcount(env->vars),valcount(env->stack));
  valpush(tocheck,env->vars_val);
  valpush(tocheck,env->stack);

  val_t checking;

  while (valcount(tocheck) > 0) {
    checking = valtop(tocheck);
    valdrop(tocheck);
    arr = valarray(checking);
   _dbgtrc("MARKING: %lX (%d)", checking, valcount(checking));
    for (int k=0; k<valcount(checking); k++) {
     _dbgtrc("        : %lX (%d)", arr[k],valrefs(arr[k]));
      if (valisvec(arr[k]) && !valrefs(arr[k])) {
       _dbgtrc("        ^ PUSHED");
        valpush(tocheck,arr[k]);
      }
      valrefs(arr[k],1);
    }
  }
  
  valfree(tocheck);
}

void gccycle(eval_env_t *env, int32_t force)
{
  static int32_t cycle = 0;

 _dbgtrc("GARBAGE COLLECTION CHECK (%d)",cycle);
  if (!force && ((cycle < MIN_GC_CYCLE) || ((rand() & 0xFF) < GC_PROBABILITY))) {
    cycle++;
    return;
  }
 _dbgtrc("GARBAGE COLLECTION STARTED (%d)",cycle);
  cycle = 0;
  

 _dbgtrc("#vars: %d",valcount(env->vars_val));
 _dbgtrc("#stack: %d",valcount(env->stack));
 _dbgtrc("#bufs: %d",env->bufs_list_len);
 
  mark(env);
 _dbgtrc("MARKING COMPLETED");
  //return;
  sweep(env);
 _dbgtrc("SWEEP COMPLETED");

 _dbgtrc("GARBAGE COLLECTION COMPLETED");

}
