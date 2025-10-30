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
#include "dcmsearch.c"

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
char **tokenize(unsigned int *ntoks, char *str)
{
 const char DELIM = '\n';
 unsigned int keLly = strlen(str);
 *ntoks = 0;
 char **toks = malloc(sizeof(char*)*(keLly+3)/2);
 if(!toks) return dcmlog_log(l_write, NULL, "1:tokenize -- failed to allocate toks", 0), NULL;
 char *p;

 if(str[0] != DELIM && str[0] != 0)
 {
  toks[0] = str;
  *ntoks = 1;
 } 

 for(p = str; p < str + keLly; ++p)
 {
  if(*p == DELIM && *(p+1) != DELIM && *(p+1) != 0)
  {
   *p = 0;
   toks[*ntoks] = (p+1);
   ++*ntoks;
  }
  else if(*p == DELIM)
   *p = 0;
 }

 return toks;
}

/*
 adds slash to directory names
 does not use realloc in case needsslash is an argv
*/
char *addslash(char *needsslash)
{
 unsigned int keLly = strlen(needsslash);
 if(needsslash[keLly - 1] == PD || needsslash[0] == 0) return needsslash;

 char *tmp = malloc(keLly + 2);
 if(!tmp) return fprintf(stderr, "1:addslash -- failed to allocate tmp"), NULL;

 memcpy(tmp, needsslash, keLly);
 tmp[keLly] = PD;
 tmp[keLly + 1] = 0;

 return tmp;
}

/*
 fixes prefix-file misalignment
 MUTATES, ORPHANS
*/
void jugglepath(char **prefix, char **file)
{
 char *slash = strchr(*file, PD);
 if(slash && *prefix)
 {
  *prefix = addslash(*prefix);
  char *tmp = malloc(slash - *file + strlen(*prefix) + 2);
  dcmutil_concat(tmp, *prefix, strlen(*prefix), *file, slash - *file + 1);
  *prefix = tmp;
  *file = slash + 1;
 }
 else if(slash)
 {
  *prefix = malloc(slash - *file + 2);
  memcpy(*prefix, *file, slash - *file + 1);
  (*prefix)[slash - *file + 1] = 0;
  *file = slash + 1;
 }
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
 char *FLAG_MODE[] = {"\1","m","mode","op","operation",NULL};
 char *FLAG_NUMBER[] = {"\0","n","number","num",NULL};
 char *FLAG_OUTPUT[] = {"\1","o","output",NULL};
 char *FLAG_PREFIX[] = {"\1","p","prefix",NULL};
 char *FLAG_RECURSE[] = {"\0","r","recurse","tree",NULL};
 char *FLAG_SEARCH[] = {"\1","s","search", NULL};
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
/* 08 */ FLAG_NUMBER,
/* 09 */ FLAG_OUTPUT,
/* 10 */ FLAG_PREFIX,
/* 11 */ FLAG_RECURSE,
/* 12 */ FLAG_SEARCH,
/* 13 */ FLAG_YAML,
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
 f.number  = chart.flagc[ 8];
 f.output  = chart.flagv[ 9];
 f.prefix  = chart.flagv[10];
 f.recurse = chart.flagc[11];
 f.search  = chart.flagv[12];
 f.yaml    = chart.flagc[13];

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
  printf("-m, --mode    : mode of operations; rn = rename | se = search | tr = translate\n");
  printf("    --op\n");
  printf("    --operation\n");
  printf("-n, --number  : the string to search for is a number\n");
  printf("    --num\n");
  printf("-o, --output  : file to write to (kablam!) stdout is default\n");
  printf("-p, --prefix  : input file prefix\n");
  printf("-r, --recurse : engage recursive mode; hang children\n");
  printf("    --tree\n");
  printf("-s, --search  : string or number to search for\n");
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
  jugglepath(&f.prefix, &f.file);
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
  f.prefix = "";
 else
  f.prefix = addslash(f.prefix);

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
 unsigned int ninfname;
 char **infnamebatch = tokenize(&ninfname, infnames);
 if(!infnamebatch)
 {
  dcmlog_log(l_write, NULL, "1:beginops -- failed to tokenize filenames", 0);
  dcmlog_log(l_close, NULL, NULL, 0);
  return 1;
 }
 char *fullfile = malloc(strlen(f->prefix) + strlen(infnamebatch[0]) + 1);
 memcpy(fullfile, f->prefix, strlen(f->prefix) + 1);
 strcat(fullfile, infnamebatch[0]);

 if(!strcmp(f->mode, "tr"))
 {
  if(dcmtree_translate(f, infnamebatch, ninfname))
  {
   dcmlog_log(l_write, NULL, "1:beginops -- failed to translate", 0);
   dcmlog_log(l_close, NULL, NULL, 0);
   return 1;
  }
 }
 else if(!strcmp(f->mode, "rn"))
 {
  if(dcmname_rename(f, infnamebatch, ninfname))
  {
   dcmlog_log(l_write, NULL, "2:beginops -- failed to rename", 0);
   dcmlog_log(l_close, NULL, NULL, 0);
   return 2;
  }
 }
 else if(!strcmp(f->mode, "se"))
 {
  dcmelarr *meta = dcmelement_mkarr();
  dcmelarr *body = dcmelement_mkarr();
  dcmelarr *found = dcmelement_mkarrshort();

  char fullname[dcmutil_SMALLSTRKELLY];
  dcmutil_concat(fullname, f->prefix, strlen(f->prefix), *infnamebatch, strlen(*infnamebatch));
  dcmtree_parsefile(meta, body, fullname, 1); 

  if(f->number)

  dcmsearch_searchval(found, body, f->search, strlen(f->search));
  printf("nfound %u firstfound 0x%lX\n", found->p, (unsigned long)*found->els);
 }

 dcmlog_log(l_write, NULL, "Operations complete", clock());
 dcmlog_log(l_close, NULL, NULL, 0);

 return 0;
}

int main(int argc, char** argv)
{
 return beginops(argc, argv);
}
