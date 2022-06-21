#include "xirnol.h"
#include "math.h"

static char *N_string="null";
static char *F_string="false";
static char *T_string="true";

void die(char *err)
{
  fprintf(stderr,"FATAL ERROR: %s\n",err);
  exit(1);
}

static int32_t findvar(val_t vars, char *var)
{
  val_t varname = val(var);
  val_t *arr;
  int32_t i = 0;
  int32_t j = -1;
  int32_t m = -1;
  int cmp;

  j = valcount(vars) -1;
  arr = valarray(vars);
  while (i<=j) {
    m = (i+j)/2;
    cmp = varcmp(&varname,arr+m);
    if (cmp == 0) return m;
    if (cmp < 0) j = m-1;
    else i = m+1;
  }

  return -1;
}

static int isstring(val_t a) { return (valisstr(a) || valisbuf(a));}
static int isnumber(val_t a) {return valisint(a) || valisdbl(a);}

static char *cast2str(val_t a, char *num)
{
    if (a == valtrue)  return T_string;
    if (a == valfalse) return F_string;
    if (a == valnil)   return N_string;
    if (isstring(a))   return valtostr(a);
    
    if (valisdbl(a)) sprintf(num,"%f",valtodbl(a));
    else if (valisint(a)) sprintf(num,"%d",valtoint(a));
    return num;
}

static int32_t cast2int(val_t x)
{
  if (valisdbl(x)) return (int32_t)(valtodbl(x));
  if (valisint(x)) return valtoint(x);
  if (isstring(x)) return atoi(valtostr(x));
  if (x == valtrue) return 1;
  return 0;
}

static double cast2dbl(val_t x)
{
  if (valisdbl(x)) return valtodbl(x);
  if (valisint(x)) return valtodbl(x);
  if (isstring(x)) return atof(valtostr(x));
  if (x == valtrue) return 1.0;
  return 0.0;
}

static val_t cast2bool(val_t x)
{
  if (x == valtrue || x == valfalse) return x;
  if (x == valnil)   return valfalse;
  if (isstring(x)) {
    char *p = valtostr(x);
    return (p != NULL && *p != '\0')?valtrue:valfalse;
  }
  if (valisint(x)) return (valtoint(x)?valtrue:valfalse);
  return valfalse;
}

static int isfalse(val_t a) { return cast2bool(a) == valfalse; }

static val_t isless(val_t a, val_t b)
{
  int32_t n=0;
  char num[32];
  _dbgtrc("< %lX %lX",a,b);

       if (valisdbl(a))  n = (valtodbl(a) < cast2dbl(b));
  else if (valisint(a))  n = (valtoint(a) < cast2int(b));
  else if (isstring(a))  n = (strcmp(valtostr(a),cast2str(b,num))<0);
  else if (valisbool(a)) n = ((a == valfalse) && (cast2bool(b) == valtrue));
  else die("can only compare numbers, strings and booleans");
  return n?valtrue:valfalse;
}

static val_t isgreater(val_t a, val_t b)
{
  int32_t n=0;
  char num[32];
  _dbgtrc("> %lX %lX",a,b);

       if (valisdbl(a))  n = (valtodbl(a) > cast2dbl(b));
  else if (valisint(a))  n = (valtoint(a) > cast2int(b));
  else if (isstring(a))  n = (strcmp(valtostr(a),cast2str(b,num))>0);
  else if (valisbool(a)) n = ((a == valtrue) && (cast2bool(b) == valfalse));
  else die("can only compare numbers, strings and booleans");
  return n?valtrue:valfalse;
}

static val_t isequal(val_t a, val_t b)
{
  int n = 0;
  if (isstring(a) && isstring(b)) {
    n = (strcmp(valtostr(a),valtostr(b))==0);
  }
  else n = (a == b);
  return n?valtrue:valfalse;
}

static void retval(val_t stack,int32_t del, val_t ret)
{
  if (del>0) valdrop(stack,del);
  valpush(stack,ret);
}

static void assignvar(eval_env_t *env)
{
  val_t a,b;
  int32_t n=-1;
  char num[32];

  a = valtop(env->stack,-2); // value
  b = valtop(env->stack);    // variable

  if (isstring(b)) n = findvar(env->vars, cast2str(b,num));
  if (n >= 0) b = val(n);
  
  if (!valisint(b)) die("Can't assign");

  valdrop(env->stack); // remove variable name
  valset(env->vars_val,b,a);
}

static val_t addbuf(eval_env_t *env)
{
  val_t buf;
 _dbgtrc("ADDING BUF");
  gccycle(env);
  if (env->bufs_free != valnil) {
    buf = env->bufs_free;
   _dbgtrc("RECYCLING %lX (s:%d c:%d)",buf,valsize(buf),valcount(buf));
    env->bufs_free = valaux(buf);
    valcount(buf,0);
  }
  else {
    buf = valbuf(20);
   _dbgtrc("NEWBUF %lX",buf);
  }

  valaux(buf,env->bufs);
  env->bufs = buf;
  env->bufs_list_len++;
  return buf;
}

static void dofunc_0(eval_env_t *env, char f)
{
  switch (f) {
    case 'T' : retval(env->stack,0,valtrue);  break;
    case 'N' : retval(env->stack,0,valnil);   break;
    case 'F' : retval(env->stack,0,valfalse); break;
    case 'R' : retval(env->stack,0,val(rand() & 0x7FFF)); break;
    case 'P' : { val_t prompt = addbuf(env);
                 int k = valbufgets(prompt,stdin);
                 char *arr=valtostr(prompt);
                 while (k>0 && (arr[k-1] == '\n' || arr[k-1] == '\r')) {
                   arr[--k] = '\0'; 
                   valcount(prompt,k);
                 }
                _dbgtrc("PROMPT: k=%d",k);
                 retval(env->stack,0,prompt);
               }
  }
}

static void dofunc_1(eval_env_t *env, char f)
{
  val_t a;
  int32_t n;
  char num[32];
  char *p;

  a = valtop(env->stack);
               
  switch (f) {
    case 'Q' : exit(cast2int(a));

    case 'D' :_dbgtrc("D: %lX",a);
               if (valisdbl(a)) printf("Number(%f)\n",valtodbl(a));
               else if (valisint(a)) printf("Number(%d)\n",valtoint(a));
               else if (a == valnil) printf("Null()\n");
               else if (a == valtrue) printf("Boolean(true)\n");
               else if (a == valfalse) printf("Boolean(false)\n");
               else if (isstring(a)) printf("String(%s)\n",valtostr(a));
               break;

    case '~' :      if (valisdbl(a)) retval(env->stack,1,val(valtodbl(a) * -1.0));
               else if (valisint(a)) retval(env->stack,1,val(valtoint(a) * -1));
               else {
                double d;
                int32_t n;
                d = cast2dbl(a);
                n = cast2int(a);
                dbgtrc("~ n: %d d: %f == : %d",n,d,(double)n == d);
                if ((double)n == d) retval(env->stack,1,val(valtoint(a) * -1));
                else retval(env->stack,1,val(valtodbl(a) * -1.0)); 
               }
               break;

    case '$' : retval(env->stack,1,val(cast2int(a))); 
               break;

    case '!' : if (valisstr(a)) {
                 n = valtostr(a)[0];
               }
               else n = valtoint(a);
              _dbgtrc("!:%d",n);
               retval(env->stack,1,n ? valfalse : valtrue);
               break;

    case 'L' : n = strlen(cast2str(a,num));
               if (n<0) n = 0;
               retval(env->stack,1,val(n));
               break;

    case 'A' : if (isnumber(a)) {
                 n = valtoint(a);
                 val_t buf = addbuf(env);
                 valset(buf,val(0),val(n));
                 valset(buf,val(1),val(0));
                 retval(env->stack,1,buf);
               }
               else if (isstring(a)) {
                 n = *valtostr(a);
                 retval(env->stack, 1, val(n));
               }
               else die("Only string or number allowed for A");
               break;

    case 'O' : p = cast2str(a,num);
               n = strlen(p);
               if (n>0 && p[n-1] == '\\') n--;
               fprintf(stdout, "%.*s",n,p); 
               if (p[n] != '\\') fputc('\n',stdout);
               fflush(stdout);
               retval(env->stack,1,valnil);
               break;

    case 'V' : n = -1;
               if (isstring(a)) n = findvar(env->vars, cast2str(a,num));
               if (n >= 0)
                 retval(env->stack,1,valget(env->vars_val,val(n)));
               else
                 retval(env->stack,1,valnil);
               break;

    case '`' : { char *cmd;
                 char num[32];
                 cmd = cast2str(a,num);
                 if (*cmd) {
                     val_t buf;
                     FILE *fp;
                     buf = valbuf(500);
                     fp = popen(valtostr(a),"r");
                     valbufreadfile(buf,fp);
                    _dbgtrc("Shell Result (%d):\n%s\n",valcount(buf),valtostr(buf));
                     pclose(fp);
                     retval(env->stack,1,buf);
                 }
                 else retval(env->stack, 1, valnilstr);
               }
               break;
  }
}

#define number_OP(name,op,a,b) \
           static val_t number_ ## name(val_t a, val_t b) \
           { \
             if (valisdbl(a)) \
               return val(valtodbl(a) op cast2dbl(b)); \
             return val(valtoint(a) op cast2int(b)); \
           }

number_OP(add,+,a,b)
number_OP(sub,-,a,b)
number_OP(mul,*,a,b)
number_OP(div,/,a,b)

static void dofunc_2(eval_env_t *env, char f)
{
  val_t a, b;

  a = valtop(env->stack,-2);
  b = valtop(env->stack);
               
  switch (f) {

    case '+' : if (isstring(a)) {
                 char *p;
                 char num[32];
                 p = cast2str(b,num);
                 val_t buf = addbuf(env);
                 valbufcpy(buf,valtostr(a));
                 valbufcat(buf,p);
                 retval(env->stack,2,buf);
               }
               else if (isnumber(a)) retval(env->stack, 2, number_add(a,b));
               else die("First argument of addition must be a number or a string");
               break;

    case '-' : if (!isnumber(a)) die("Subtraction only accepts numbers as first argument");
               retval(env->stack, 2, number_sub(a,b));
               break;

    case '*' : if (isstring(a)) {
                 val_t buf = addbuf(env);
                 valtostr(buf)[0] = '\0';
                 for (int k= cast2int(b); k>0; k--)
                   valbufcat(buf,valtostr(a));
                 retval(env->stack,2,buf);
               }
               else if(isnumber(a)) retval(env->stack, 2,number_mul(a,b));
               else die("First argument of multiplication must be a number or a string");
               break;

    case '/' : if (!isnumber(a)) die("Division only accepts numbers as first argument");
               retval(env->stack, 2, number_div(a,b));
               break;

    case '%' : { if (valisdbl(a)) 
                   retval(env->stack, 2, val(fmod(valtodbl(a) , cast2dbl(b))));
                 else if (valisint(a))
                   retval(env->stack, 2, val(valtoint(a) % cast2int(b)));
                 else die("Modulo only accepts numbers as a base");
               }
               break;

    case '^' : if (valisint(a)) {
                 int32_t base = valtoint(a);
                 int32_t expn = cast2int(b);
                 int32_t powr = 1;
                 if (base == 1) powr = 1;
                 else if (expn < 0) {
                   powr = 0;
                   if (base == 0) die("cant rise 0 to negative");
                   if (base == -1) powr = 1 - 2 * (expn & 1);
                 }
                 else for (int k=0; k<expn;k++) powr *= base;
                 retval(env->stack, 2,val(powr));
               }
               else if (valisdbl(a)) {
                 retval(env->stack, 2,val(pow(valtodbl(a),cast2dbl(b))));
               }
               else die("Exponention base must be a number");
               break;

    case ';' : retval(env->stack,2,b);
               break;

    case '<' : retval(env->stack, 2, isless(a,b));
               break;

    case '>' : retval(env->stack, 2, isgreater(a,b));
               break;

    case '?' : retval(env->stack, 2, isequal(a,b));
               break;
  }
}

static void dofunc_3(eval_env_t *env, char f)
{
  val_t a, b, c;
  char num[32];
  char *p;
 _dbgtrc("F3: %c",f);
  if (f == 'G') {
    val_t buf;
    int32_t from;
    int32_t len;
    int32_t l;

    a = valtop(env->stack,-3);
    b = valtop(env->stack,-2);
    c = valtop(env->stack);

    p = cast2str(a, num);

    from = cast2int(b);
    len = cast2int(c);

    if (from<0) from = 0;
   _dbgtrc("GET '%s' %d %d",p,from,len);
    if (len <0) len = 0;
    l = strlen(p);
    if (from >= l || len == 0 || *p == '\0') {
      retval(env->stack, 3, valnilstr);
      return;
    }
    buf = addbuf(env);
    valbufcpy(buf,p+from,0,len);
   _dbgtrc("GETRET: %s %lX",valtostr(buf), buf);
    retval(env->stack, 3, buf);
   _dbgtrc("GETSTK: %lX",valtop(env->stack));
  }
  else die("unkonw function");
}

static void dofunc_4(eval_env_t *env, char f)
{
  val_t a, b, c, d;
  char num1[20];
  char num2[20];
  char *p1;
  char *p2;
 _dbgtrc("F4: %c",f);
  if (f == 'S') {
    val_t buf;
    int32_t from;
    int32_t len;
    int32_t l;

    a = valtop(env->stack,-4);
    b = valtop(env->stack,-3);
    c = valtop(env->stack,-2);
    d = valtop(env->stack);   

    p1 = cast2str(a, num1);
   _dbgtrc("SET A:%s",p1);
    l = strlen(p1);

    from = valtoint(b);
    if (from < 0) from = 0;
    if (from > l) from = l;

    len = valtoint(c);
    if ((from+len) > l) len = l - from;

    p2 = cast2str(d, num2);
   _dbgtrc("SET D:%s start: %d len: %d",p2,from,len);

    buf = addbuf(env);
 
    valbufcpy(buf,p1,0,from);
   _dbgtrc("SET1: %s",valtostr(buf));
    valbufcat(buf,p2);
   _dbgtrc("SET2: %s",valtostr(buf));
    valbufcat(buf,p1+from+len);
   _dbgtrc("SET3: %s",valtostr(buf));

    retval(env->stack, 4, buf);
  }
  else die("unkonw function");
}

static val_t string_const(val_t stack, char *start, int32_t len)
{
  start[len] = '\0';
  return val(start);
}

#define NODE_OFFSET 1

val_t kneval(ast_t astcur)
{

  int32_t curnode = astroot(astcur);
  val_t top ;
  char *start;

  eval_env_t env;

  env.stack     = valvec(100);
  env.vars      = valnil;
  env.vars_val  = valnil;
  env.bufs      = valnil;
  env.bufs_free = valnil;
  env.bufs_list_len = 0;

  srand(time(0));

  if (astaux(astcur) == NULL) return -1;
  env.vars = *((val_t *)astaux(astcur));
  
  if (valisvec(env.vars)) {
    env.vars_val = valvec(valcount(env.vars));
    val_t *a = valarray(env.vars_val);
    if (a == NULL) die("Unexpected!");
    for (int k=0; k<valcount(env.vars); k++) 
      a[k] = valnil;
  }

  while (curnode != ASTNULL ) {
    if (astisnodeentry(astcur,curnode)) {
      start = astnodefrom(astcur,curnode);
     _dbgtrc("NODE: %d",curnode);
      if (astnodeis(astcur,curnode,integer)) {
       _dbgtrc("NUM: %d", atoi(start));
        retval(env.stack,0,val(atoi(start)));
      }
      else if (astnodeis(astcur,curnode,float)) {
       _dbgtrc("NUM: %d", atof(start));
        retval(env.stack,0,val(atof(start)));
      }
      else if (astnodeis(astcur,curnode,string)) {
        retval(env.stack,0,string_const(env.stack,start,astnodelen(astcur,curnode)));
      }
      else if (astnodeis(astcur,curnode,variable)) {
        int v = findvar(env.vars,start);
       _dbgtrc("VAR: %d (%.4s)", v,start);
        retval(env.stack,0,valget(env.vars_val,val(v)));
      }
      else if (astnodeis(astcur,curnode,varref)) {
        int v = findvar(env.vars,start);
       _dbgtrc("VARREF: %d (%.4s)", v,start);
        retval(env.stack,0,val(v));
      }
      else if (astnodetag(astcur,curnode) >= 0xF0) { // It's a function!
       _dbgtrc("FNC: %c ", *start);
        switch (astnodetag(astcur,curnode)) {
          case 0xF0: dofunc_0(&env,*start); break;
          case 0xF1: dofunc_1(&env,*start); break;
          case 0xF2: dofunc_2(&env,*start); break;
          case 0xF3: dofunc_3(&env,*start); break;
          case 0xF4: dofunc_4(&env,*start); break;
          default:   die("unknown function");
        }
      }
      else if (astnodeis(astcur,curnode,assign)) {
        if (env.vars_val == valnil) die("No variable defined/referenced");
        assignvar(&env);
      }
      else if (astnodeis(astcur,curnode,while)) {
        retval(env.stack,0,valnil); // starting value
      }
      else if (astnodeis(astcur,curnode,while_check)) {
        val_t a = valtop(env.stack);
        valdrop(env.stack);
       _dbgtrc("WHILE_CHECK drop: %lX",a);
        if (isfalse(a)) curnode = astlast(astcur,curnode);
        else valdrop(env.stack);
      }
      else if (astnodeis(astcur,curnode,while_end)) {
       _dbgtrc("WHILE_end drop: %lX",valtop(env.stack));
        curnode = astup(astcur,curnode);
      }
      else if (astnodeis(astcur,curnode, if_then)) {
        val_t a = valtop(env.stack);
        valdrop(env.stack);
        if (isfalse(a)) {
          curnode = astright(astcur,curnode);
          curnode = astright(astcur,curnode);
        }
      }
      else if (astnodeis(astcur,curnode,if_else)) {
          curnode = astright(astcur,curnode);
          curnode = astright(astcur,curnode);
      }
      else if (astnodeis(astcur,curnode, and_check)) {
        val_t a = valtop(env.stack);
        if (isfalse(a)) {
          curnode = astright(astcur,curnode);
          curnode = astright(astcur,curnode);
        }
      }
      else if (astnodeis(astcur,curnode, or_check)) {
        val_t a = valtop(env.stack);
        if (!isfalse(a)) {
          curnode = astright(astcur,curnode);
          curnode = astright(astcur,curnode);
        }
      }
      else if (astnodeis(astcur,curnode,block)) {
        val_t addr = valconst(NODE_OFFSET,astdown(astcur,curnode));
        curnode = astright(astcur,curnode);
        retval(env.stack,0,addr);
      }
      else if (astnodeis(astcur,curnode,block_ret)) {
        val_t a = valtop(env.stack,-2);
        val_t b = valtop(env.stack);
       _dbgtrc("BLOCK RETURN: %lX",a);
        retval(env.stack,2,b);
        if (!valisconst(NODE_OFFSET,a)) die("invalid return");
        curnode = valtoint(a);
      }
      else if (astnodeis(astcur,curnode,call)) {
        val_t a = valtop(env.stack);
        if (!valisconst(NODE_OFFSET,a)) die("invalid block");
        val_t r = valconst(NODE_OFFSET,curnode);
        retval(env.stack,1,r);
       _dbgtrc("CALL %lX ret: %lX",a,r);
        curnode = valtoint(a);
      }
    }

    curnode = astnext(astcur,curnode);
  }
  top = valtop(env.stack);
  
 _dbgtrc("FINAL DEPTH: %d",valcount(env.stack));

  valfree(env.vars_val);
  valfree(env.stack);
  
  for (val_t p = env.bufs; p != valnil; p=env.bufs ) {
    env.bufs = valaux(p);
    valfree(p);
  }
  for (val_t p = env.bufs_free; p != valnil; p=env.bufs_free ) {
    env.bufs_free = valaux(p);
    valfree(p);
  }

  return top;
}
