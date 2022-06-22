#include "xirnol.h"

/*
A very simple mark&sweep GC
Only buffers need to be collected and we have a list of 
buffers in env.bufs.

The only alive buffers are those referenced in the stack
or in one in the variables. So we simply:

   - scan the stack and set the MSB of the size filed for any buffers referenced
   - scan the variables and do the same
   - scan the buf list, move to the free list the ones that are not marked
     and clear the MSB for those that are marked.

A GC cycle will be started when the number of allocated buffers is greater
(by a factor K) than the number of variables plus the depth of the stack.

*/

void gccycle(eval_env_t *env)
{
  val_t *arr ;
  return;
 _dbgtrc("GARBAGE COLLECTION CHECK");
 _dbgtrc("#vars: %d",valcount(env->vars_val));
 _dbgtrc("#stack: %d",valcount(env->stack));
 _dbgtrc("#bufs: %d",env->bufs_list_len);

  if (env->bufs_list_len > 2 * (valcount(env->vars)+valcount(env->stack))) {

   _dbgtrc("GARBAGE COLLECTION STARTED");
   _dbgtrc("Mark buffers in vars");
    arr = valarray(env->vars_val);
    for (int k=0; k<valcount(env->vars_val); k++) {
      if (valisbuf(arr[k])) valrefs(arr[k],1000+k);
    }

   _dbgtrc("Mark buffers in stack");
    arr = valarray(env->stack);
    for (int k=0; k<valcount(env->stack); k++) {
      if (valisbuf(arr[k])) valrefs(arr[k],2000+k);
    }

   _dbgtrc("Move buffers to the free list");
    val_t *parent;
    val_t  child;
    parent = &env->bufs;

    while ((child = *parent) != valnil) {
     _dbgtrc("BUF: %d",valrefs(child));
      if (valrefs(child) > 0) { // MARKED
        valrefs(child,0);
        parent = valtoptr(child); //Only works because aux is the first field!
      }
      else { // Move to free list
        *parent = valaux(child); // Remove from list
        env->bufs_list_len--;
        valaux(child,env->bufs_free); // Add to the free list
        valcount(child,0);
        env->bufs_free = child;
      }
    }

   _dbgtrc("GARBAGE COLLECTION COMPLETED");
  }
}
