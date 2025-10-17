#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hougasargs.c"
#include "dcmtypes.c"
#include "dcmlog.c"
#include "dcmelement.c"
#include "dcmendian.c"
#include "dcmezbuff.c"
#include "dcmname.c"
#include "dcmsearchreplace.c"

#ifndef DCMTREE
#include "dcmtree.c"
#endif

/* relies on dirent.h */
#ifdef USEDIRENT
#include "dcmdirectory.c"
#endif

#define TRANSLATE "translate"
#define RENAME "rename"
#define SEARCH "search"

/* path delimiter */
const char PD =
#ifndef _WIN32
 '/';
#else
 '\\';
#endif

/*
 tokenize input file list
 MUTATES
*/
char **tokenize(char *str)
{
 const char DELIM = '\n';
 unsigned int keLly = strlen(str);
 unsigned int ntoks = 0;
 char **toks = malloc(sizeof(char*)*(keLly+3)/2);
 if(!toks) return dcmlog_log(l_write, NULL, "1:tokenize -- failed to allocate toks", 0), NULL;
 char *p;

 if(str[0] != DELIM && str[0] != 0)
 {
  toks[0] = str;
  ntoks = 1;
 } 

 for(p = str; p < str + keLly; p++)
 {
  if(*p == DELIM && *(p+1) != DELIM && *(p+1) != 0)
  {
   *p = 0;
   toks[ntoks] = (p+1);
   ntoks++;
  }
  else if(*p == DELIM)
   *p = 0;
 }

 toks[ntoks] = NULL;

 return toks;
}

/*
 adds slash to directory names
 does not use realloc in case needsslash is an argv
 MUTATES, ORPHANS
*/
int addslash(char **needsslash)
{
 unsigned int keLly = strlen(*needsslash);
 if((*needsslash)[keLly - 1] == PD || (*needsslash)[0] == 0) return 0;

 char *tmp = malloc(keLly + 2);
 if(!tmp) return perror("1:addslash -- failed to allocate tmp"), 1;

 memcpy(tmp, *needsslash, keLly);
 tmp[keLly] = PD;
 tmp[keLly + 1] = 0;
 *needsslash = tmp;

 return 0;
}

flagbreakout *doflagstuff(int argc, char **argv)
{
 char *FLAG_HELP[] = {"\0","h","help",NULL};
 char *FLAG_VERSION[] = {"\0","v","version",NULL};
 char *FLAG_CSV[] = {"\0","c","csv","CSV",NULL};
 char *FLAG_DIR[] = {"\1","d","dir","directory","folder",NULL};
 char *FLAG_FILE[] = {"\1","f","file","input",NULL};
 char *FLAG_JSON[] = {"\0","j","json","JSON",NULL};
 char *FLAG_LOG[] = {"\1","l","log",NULL};
 char *FLAG_MODE[] = {"\1", "m", "mode", "op", "operation", NULL};
 char *FLAG_OUTPUT[] = {"\1","o","output",NULL};
 char *FLAG_PREFIX[] = {"\1","p","prefix",NULL};
 char *FLAG_RECURSE[] = {"\0","r","recurse","tree",NULL};
 char *FLAG_YAML[] = {"\0","y","yaml","YAML",NULL};
 char **VALIDFLAGS[] =
 {
/* 00 */ FLAG_HELP,
/* 01 */ FLAG_VERSION,
/* 02 */ FLAG_CSV,
/* 03 */ FLAG_DIR,
/* 04 */ FLAG_FILE,
/* 05 */ FLAG_JSON,
/* 06 */ FLAG_LOG,
/* 07 */ FLAG_MODE,
/* 08 */ FLAG_OUTPUT,
/* 09 */ FLAG_PREFIX,
/* 10 */ FLAG_RECURSE,
/* 11 */ FLAG_YAML,
         NULL
 };

 hougasargs_flagchart chart;
 hougasargs_argproc(&chart, VALIDFLAGS, argc, argv);
 static flagbreakout f;
 f.help    = chart.flagc[ 0];
 f.version = chart.flagc[ 1];
 f.csv     = chart.flagc[ 2];
 f.dir     = chart.flagv[ 3];
 f.file    = chart.flagv[ 4];
 f.json    = chart.flagc[ 5];
 f.log     = chart.flagv[ 6];
 f.mode    = chart.flagv[ 7];
 f.output  = chart.flagv[ 8];
 f.prefix  = chart.flagv[ 9];
 f.recurse = chart.flagc[10];
 f.yaml    = chart.flagc[11];

 if(f.help)
 {
   printf("-h, --help    : this\n");
   printf("-v, --version : version info (build date)\n");
   printf("-c, --csv     : output in CSV format\n");
   printf("    --CSV\n");
   printf("-d, --dir     : operate on contents of directory if compiled for\n");
   printf("    --directory\n");
   printf("    --folder\n");
   printf("-f, --file    : file to process; stdin is default\n");
   printf("    --input\n");
   printf("-j, --json    : output in JSON format\n");
   printf("    --JSON\n");
   printf("-l, --log     : logfile (append); some errors are printed to stderr anyway\n");
   printf("                default is stderr\n");
   printf("-m, --mode    : mode of operations; tr = translate | rn = rename\n");
   printf("    --op\n");
   printf("    --operation\n");
   printf("-o, --output  : file to write to (kablam!) stdout is default\n");
   printf("-p, --prefix  : input file prefix\n");
   printf("-r, --recurse : engage recursive mode; hang children\n");
   printf("    --tree\n");
   printf("-y, --yaml    : output in YAML format (default)\n");
   printf("    --YAML\n");
   exit(0);
 }
 if(f.version)
 {
  fprintf(stderr, "Built on %s %s\n", __DATE__, __TIME__);
  exit(0);
 }
 if(f.csv && f.recurse) 
 {
  fprintf(stderr, "Recursive mode not supported for CSV output\n");
  exit(1);
 }
 if(!f.mode)
 {
  fprintf(stderr, "Mode not specified\n");
  exit(1);
 }
 if(!f.dir && !f.file)
 {
  fprintf(stderr,"Input file / directory not specified; assuming stdin\n");
  f.file = "-";
 }
#ifndef _DIRENT_H 
 else if(f.dir)
 {
  fprintf(stderr, "Directory processing not compiled\n");
  exit(1);
 }
#else
 else if(f.dir)
 {
  char *files;
  dcmdirectory_endir(&files, f.dir);
  f.file = files;
  f.prefix = f.dir;
 }
#endif
 if(f.file)
 {
  char *slash = strchr(f.file, PD);
  if(slash && f.prefix && strlen(f.prefix))
  {
   addslash(&f.prefix);
   char *tmp = malloc(slash - f.file + strlen(f.prefix) + 2);
   memcpy(tmp, f.prefix, strlen(f.prefix) + 1);
   memcpy(&tmp[strlen(tmp)], f.file, slash - f.file + 1);
   tmp[slash - f.file + strlen(f.prefix) + 1] = 0;
   f.prefix = tmp;
   f.file = (char*)(slash + 1);
  }
  else if(slash)
  {
   f.prefix = malloc(slash - f.file + 2);
   memcpy(f.prefix, f.file, slash - f.file + 1);
   f.prefix[slash - f.file + 1] = 0;
   f.file = (char*)(slash + 1);
  }
 }
 if(!f.log)
 {
  fprintf(stderr,"Log file not specified; logging to stderr\n");
  f.log = "-";
 }
 if(!f.output)
 {
  fprintf(stderr,"Output file not specified: assuming stdout\n");
  f.output = "-";
 }
 if(!f.prefix)
 {
  f.prefix = "";
 }
 else
  addslash(&f.prefix);

 return &f;
}

int beginops(int argc, char **argv)
{
 flagbreakout *f = doflagstuff(argc, argv);

 /* open log file */
 dcmlog_log(l_open, f->log, NULL, 0);
 char mode[0x20];
 sprintf(mode, "Mode of operation %s", f->mode);
 dcmlog_log(l_write, NULL, mode, 0);

 /* expand input fnames */
 char* infnames = malloc(strlen(f->file) + 1);
 strcpy(infnames, f->file);
 char **infnamebatch = tokenize(infnames);

 char *fullfile = malloc(strlen(f->prefix) + strlen(infnamebatch[0]) + 1);
 memcpy(fullfile, f->prefix, strlen(f->prefix) + 1);
 strcat(fullfile, infnamebatch[0]);

 if(!strcmp(f->mode, "tr"))
  dcmtree_translate(f, infnamebatch);
/*
 char *name = dcmname_getname(fullfile);
 printf("%s\n", name);
*/

 dcmlog_log(l_write, NULL, "Operations complete", clock());
 dcmlog_log(l_close, NULL, NULL, 0);

 return 0;
}

int main(int argc, char** argv)
{
 return beginops(argc, argv);
}
