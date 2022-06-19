#define DEBUG DEBUG_TEST
#define VAL_MAIN
#define SKP_MAIN
#define VRG_MAIN
#include "vrg.h"

#include "xirnol.h"

/************************************/

#define MAXFNAME 120
static char fnamebuf[MAXFNAME+8];
static char bnamebuf[MAXFNAME];

static char *loadsource(char *fname)
{
  FILE *f;
  int32_t size=0;
  char *src=NULL;

  f = fopen(fname,"rb");
  if (f) {
    fseek(f,0,SEEK_END);
    size = ftell(f);
    fseek(f,0,SEEK_SET);
    src = malloc(size+1+!(size & 1));
    if (src) {
      *src = '\0';
      if (fread(src,size,1,f)) {
        src[size] = '\0';
      }
      else {
        free(src);
        src = NULL;
      };
    }
    fclose(f);
  }
  
  return src;
}

#define trace(...) (fprintf(stderr,__VA_ARGS__),fputc('\n',stderr))

static int isvarchr(char c)
{
  return (c=='_') || (('a'<=c) && (c<='z'));
}

int varcmp(const void *a, const void *b)
{
  char *pa = valtostr(*((val_t *)a));
  char *pb = valtostr(*((val_t *)b));

 _dbgtrc("'%c' '%c'",*pa,*pb);
  while (isvarchr(*pa)) {
    if (!isvarchr(*pb)) return 1;
    if (*pa != *pb) return (*pa - *pb);
    pa++;
    pb++;
  }

  if (isvarchr(*pb)) return -1;
  return 0;
}

static void fixvars(val_t v)
{
  if (!valisvec(v) || valcount(v) == 0) return;
  val_t *arr;

  arr = valarray(v);

  qsort(arr,valcount(v),sizeof(val_t),varcmp);

  int j = 0;
  int k = 1;

  while (k < valcount(v)) {
    if (varcmp(arr+j, arr+k) != 0) {
      j++;
      if (j != k) arr[j] = arr[k];
    }
    k++;
  }

  valcount(v,j+1);

//  char *s;
//  for (k=0; k< valcount(v); k++) {
//    s = valtostr(arr[k]);
//    while (isvarchr(*s)) 
//      fputc(*s++,stderr);
//    fputc('\n',stderr);
//  }
}


int main(int argc, char *argv[])
{
  char *knbuf = NULL;
  ast_t ast = NULL;
  FILE *src=NULL;
  FILE *hdr=NULL;
  char *s;
  int trc =0;
  //int argn;
  char *fname="_expr_";

  vrgver("xirnol (a knight interpeter)\nv0.0.7-beta (C) 2022 Remo Dentato http://gh/dentato.com/xirnol");

  vrgoptions(argc,argv) {
    vrgopt("-t       ","Enable tracing in parser") {
      trc = 1;
    }

    vrgopt("-f filename", "Run source file") {
        fname = vrgoptarg;
        knbuf = loadsource(fname);
    }

    vrgopt("-e 'expr'", "Evaluate expression") {
        knbuf = strdup(vrgoptarg);
    }

    vrgopt("-h       ", "Print help and exit") {
        vrghelp();
    }
  }

  if (knbuf == NULL && vrgargn<argc) {
    fname = argv[vrgargn];
    knbuf = loadsource(fname);
  }

  if (knbuf == NULL) vrgerror("Missing file or expression");

  ast = skpparse(knbuf,prog,trc);

  if (asthaserr(ast)) {
    trace("In rule: '%s'",asterrrule(ast));
    char *ln = asterrline(ast);
    char *endln = ln;
    while (*endln && *endln != '\r' && *endln != '\n') endln++;
    int32_t col = asterrcolnum(ast);
    trace("Error: %s",asterrmsg(ast));
    trace("%.*s",(int)(endln-ln),ln);
    trace("%.*s^",col,"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    return 1;
  }
  else {
    strcpy(bnamebuf,fname);
    s=bnamebuf;
    while (*s) s++;
    while (s>bnamebuf && s[-1] != '.') s--;
    if (s>bnamebuf) s[-1] = '\0';
    
    sprintf(fnamebuf,"%s"".t",bnamebuf);
    hdr = fopen(fnamebuf,"w");
    if (hdr) {
      astprint(ast,hdr);
      fprintf(hdr,"%d nodes\n",astnumnodes(ast));
      fclose(hdr); hdr = NULL;
      //fprintf(stderr,"AST file: '%s'\n",fnamebuf);
      val_t v = *((val_t *)astaux(ast));
      if (v != valnil) {
        fixvars(v);
       _dbgtrc("Vars: %d",valcount(v));
      }
    }

    //val_t retevl;
    //retevl = 
    kneval(ast);

   // fprintf(stderr,"RET: %lX\n",retevl);
    
#if 0
    sprintf(fnamebuf,"%s"".c",bnamebuf);
    src = fopen(fnamebuf,"w");
    if (src) {
      generate(ast,src);
      fclose(src); src = NULL;
    }
    else {
      fprintf(stderr,"Unable to write on %s\n",fnamebuf);
    }
#endif    
  }

  if (src) fclose(src);
  if (hdr) fclose(hdr);
   
  if (knbuf) free(knbuf);
  if (ast) {
    if (astaux(ast) != NULL) {
      val_t vars = *((val_t *)astaux(ast));
      valfree(vars);
    }
    astfree(ast);
  }
  return 0;
}
