#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HOUGASARGS 1

#define hougasargs_startswithdoubletac(str) ((str)[0]=='-'&&(str)[1]=='-')
#define hougasargs_startswithsingletac(str) ((str)[0]=='-'&&(str)[1]!='-')

typedef struct
{
 unsigned int *flagc;
 char **flagv;
} hougasargs_flagchart;

/*
 list for args so they can be removed easily
*/
typedef struct hougasargs_argnode
{
 char *arg;
 struct hougasargs_argnode *prev;
 struct hougasargs_argnode *next;
} hougasargs_argnode;

/*
 make a list of argc **argv
*/
hougasargs_argnode *hougasargs_listanize(int argc, char **argv)
{
 hougasargs_argnode *first = malloc(sizeof(hougasargs_argnode));
 hougasargs_argnode *current = first;
 current->prev = NULL;

 int i;
 for(i = 1; i < argc; ++i)
 {
  current->arg = argv[i];
  if(i+1 < argc)
  {
   current->next = malloc(sizeof(hougasargs_argnode));
   current->next->prev = current;
   current = current->next;
  }
 }

 current->next = NULL;

 return first;
}

void hougasargs_delarglist(hougasargs_argnode *argnode)
{
 if(!argnode) return;

 hougasargs_argnode *next = argnode->next;
 hougasargs_argnode *prev = argnode->prev;

 while(argnode)
 {
  free(argnode);
  argnode = next;
  next = !argnode ? NULL : argnode->next;
 }
 argnode = prev;
 while(argnode)
 {
  free(argnode);
  argnode = prev;
  prev = !argnode ? NULL : argnode->prev;
 }
}

/*
 remove node from list does free
*/
hougasargs_argnode *hougasargs_removenode(hougasargs_argnode *current)
{
 if(!current) return NULL;

 hougasargs_argnode *prev = current->prev;
 hougasargs_argnode *next = current->next;
 free(current);

 if(prev) prev->next = next;
 if(next) next->prev = prev;

 return next;
}

/*
 handle potentially multi-character single char flags
*/
int hougasargs_singletacflag(hougasargs_flagchart *flagchart, char ***validflags, hougasargs_argnode *argnode)
{
 if(argnode->arg[1] == 0) return 0;

 char *arg = argnode->arg;

 register unsigned int i,j;
 for(i = 1; arg[i] != 0; ++i)
 {
  for(j = 0; validflags[j] && arg[i] != validflags[j][1][0]; ++j);

  if(!validflags[j]) return fprintf(stderr, "Flag %c invalid\n", arg[i]), 1;

  ++flagchart->flagc[j];

  if(validflags[j][0][0] && arg[i+1] != 0) return fprintf(stderr, "Flag %c requires arg\n", arg[i]), 2;
 }

 if(!validflags[j][0][0]) return 0;
 if(!argnode->next) return fprintf(stderr, "Flag %c requires arg\n",arg[i-1]), 3;

 flagchart->flagv[j] = argnode->next->arg;
 hougasargs_removenode(argnode->next);

 return 0;
}

/*
 handle multi-character flags
*/
int hougasargs_doubletacflag(hougasargs_flagchart *flagchart, char ***validflags, hougasargs_argnode *argnode)
{
 char *arg = argnode->arg;
 register unsigned int i,j;
 for(i = 0; validflags[i]; ++i)
 {
  for(j = 2; validflags[i][j] && strcmp(&arg[2],validflags[i][j]); ++j);
  if(validflags[i][j]) break;
 }

 if(!validflags[i]) return printf("Flag %s invalid\n", arg), 1;

 ++flagchart->flagc[i];

 if(validflags[i][0][0] == 0) return 0;
 if(!argnode->next) return printf("Flag %s requires arg\n", validflags[i][j]), 2;

 flagchart->flagv[i] = argnode->next->arg;
 hougasargs_removenode(argnode->next);

 return 0;
}

hougasargs_argnode *hougasargs_argproc(hougasargs_flagchart *flagchart, char ***validflags, int argc, char **argv)
{
 unsigned int nvalid; for(nvalid = 0; validflags[nvalid]; ++nvalid);
 flagchart->flagc = malloc(sizeof(int)*nvalid);
 flagchart->flagv = malloc(sizeof(char*)*nvalid);

 register unsigned int i;
 for(i = 0; i<nvalid; ++i)
 {
  flagchart->flagc[i] = 0;
  flagchart->flagv[i] = NULL;
 }

 if(argc == 1) return NULL;

 unsigned int endofflags;
 for(endofflags = 1; endofflags < argc && strcmp(argv[endofflags],"--"); ++endofflags);

 hougasargs_argnode *arghead = hougasargs_listanize(endofflags, argv);
 hougasargs_argnode *current = arghead;

 while(current)
 {
  if(hougasargs_startswithdoubletac(current->arg))
  {
   if(hougasargs_doubletacflag(flagchart, validflags, current))
   {
    hougasargs_delarglist(current);
    exit(1);
   }
   current = hougasargs_removenode(current);
  }
  else if(hougasargs_startswithsingletac(current->arg) && current->arg[1] != 0)
  {
   if(hougasargs_singletacflag(flagchart, validflags, current))
   {
    hougasargs_delarglist(current);
    exit(1);
   }
   current = hougasargs_removenode(current);
  }
  else current = current->next;
 }
 
 return arghead;
}
