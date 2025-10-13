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
#include "dcmfile.c"
#include "dcmtree.c"

/* relies on dirent.h */
#ifdef USEDIRENT
#include "dcmdirectory.c"
#endif

/* tokenize input file list */
void tokenize(char ***toks, unsigned int *ntoks, char *str)
{
 const char DELIM = '\n';
 unsigned int length = strlen(str);
 *ntoks = 0;
 *toks = (char**)malloc(sizeof(char*)*((length+1)/2));
 char *p;

 if(str[0] != DELIM && str[0] != 0)
 {
  (*toks)[0] = str;
  *ntoks = 1;
 } 

 for(p = str; p < &str[length]; p++)
 {
  if(*p == DELIM && *(p+1) != DELIM && *(p+1) != 0)
  {
   *p = 0;
   (*toks)[*ntoks] = (p+1);
   ++*ntoks;
  }
  else if(*p == DELIM)
   *p = 0;
 }
}

void doflagstuff(hougasargs_flagchart *chart, int argc, char **argv)
{
 char *FLAG_HELP[] = {"\0","h","help",NULL};
 char *FLAG_VERSION[] = {"\0","v","version",NULL};
 char *FLAG_CSV[] = {"\0","c","csv","CSV",NULL};
 char *FLAG_DIR[] = {"\1","d","dir","directory","folder",NULL};
 char *FLAG_FILE[] = {"\1","f","file","input",NULL};
 char *FLAG_JSON[] = {"\0","j","json","JSON",NULL};
 char *FLAG_LOG[] = {"\1","l","log",NULL};
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
/* 07 */ FLAG_OUTPUT,
/* 08 */ FLAG_PREFIX,
/* 09 */ FLAG_RECURSE,
/* 10 */ FLAG_YAML,
 NULL
 };

 hougasargs_argproc(chart, VALIDFLAGS, argc, argv);

 if(chart->flagc[0])
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
  printf("-o, --output  : file to write to (kablam!) stdout is default\n");
  printf("-p, --prefix  : input file prefix\n");
  printf("-r, --recurse : engage recursive mode; hang children\n");
  printf("    --tree\n");
  printf("-y, --yaml    : output in YAML format (default)\n");
  printf("    --YAML\n");
  exit(0);
 }
 if(chart->flagc[1])
 {
  printf("Built on %s\n", __DATE__);
  exit(0);
 }
 if(chart->flagc[7] && chart->flagc[2]) 
 {
  printf("Recursive mode not supported for CSV output.\n");
  exit(1);
 }
 if(chart->flagv[3] == NULL && chart->flagv[4] == NULL)
 {
  fprintf(stderr,"Input file / directory not specified; assuming stdin\n");
  chart->flagv[3] = "-";
 }
#ifndef _DIRENT_H 
 else if(chart->flagc[3])
 {
  fprintf(stderr, "Directory processing not compiled\n");
  exit(1);
 }
#else
 else if(chart->flagv[3] != NULL)
 {
  char *files;
  dcmdirectory_endir(&files, chart->flagv[3]);
  chart->flagv[4] = files;
  chart->flagv[8] = chart->flagv[3];
 }
#endif
 if(chart->flagv[6] == NULL)
 {
  fprintf(stderr,"Log file not specified; logging to stderr\n");
  chart->flagv[6] = "-";
 }
 if(chart->flagv[7] == NULL)
 {
  fprintf(stderr,"Output file not specified: assuming stdout\n");
  chart->flagv[7] = "-";
 }
 if(chart->flagv[8] == NULL)
 {
  chart->flagv[8] = "";
 }
}

int beginops(int argc, char **argv)
{
 hougasargs_flagchart chart;
 doflagstuff(&chart, argc, argv);

 /* open log file */
 dcmlog_log(0, chart.flagv[6], NULL);

 /* expand input fnames */
 char *file = chart.flagv[4];
 unsigned int infnamelength = strlen(file);
 char* infnames = malloc(infnamelength+1);
 strcpy(infnames, file);
 unsigned int ninfname;
 char **infnamebatch;
 tokenize(&infnamebatch, &ninfname, infnames);

 dcmfile_translate(&chart, infnamebatch, ninfname);

 dcmlog_log(-1, NULL, NULL);

 return 0;
}

#ifdef UNDEFINED

int parsefile(int argc, char **argv)
{
 /* start of operations time */
 time_t now; time(&now);

 hougasargs_flagchart chart;
 doflagstuff(&chart, argc, argv);

 /* names for flagchart components */
 unsigned int  csv     = chart.flagc[ 2];
 /* UNUSED: handled in doflagstuff char         *dir     = chart.flagv[ 3]; */
 char         *file    = chart.flagv[ 4];
 unsigned int  json    = chart.flagc[ 5];
 char         *log     = chart.flagv[ 6];
 char         *output  = chart.flagv[ 7];
 char         *prefix  = chart.flagv[ 8];
 unsigned int  recurse = chart.flagc[ 9];
 unsigned int  yaml    = chart.flagc[10];

 /* open logfile */
 FILE* errfile = strcmp("-", log) ? fopen(log, "a") : stderr;
 if(errfile == NULL) return perror("1:parsefile"), 1;

 /* print start of operations time */
 fprintf(errfile,"%011ld  : ", now);
 struct tm *snow = gmtime(&now);
 int month = snow->tm_mon + 1;
 int year = snow->tm_year + 1900;
 fprintf(errfile,"%04d_%02d_%02d %02d:%02d:%02d Z  : Log file opened\n", year, month, snow->tm_mday, snow->tm_hour, snow->tm_min, snow->tm_sec);

 /* parse possibly multple file names */
 unsigned int infnamelength = strlen(file);
 char* infnames = (char*)malloc(infnamelength+1);
 strcpy(infnames, file);
 unsigned int ninfname;
 char **infnamebatch;
 tokenize(&infnamebatch, &ninfname, infnames);

 m_format format = yaml ? f_yaml :
                   json ? f_json :
                   csv  ? f_csv  :
                          f_yaml ;
 FILE *outfile = strcmp(output, "-") ? fopen(output, "w") : stdout;
 if(outfile == NULL)
 {
  fprintf(errfile, " ERROR 2: failed to open output file %s\n", log);
  if(errfile != stderr) fclose(errfile);
  return 2;
 }
 outmode omode =
 {
  format,
  recurse,
  outfile,
  "",
  0,
  ninfname - 1
 };

 /* memory for time data */
 clock_t *inputloaded    = (clock_t*)malloc(sizeof(clock_t)*ninfname);
 clock_t *fileprocessed  = (clock_t*)malloc(sizeof(clock_t)*ninfname);
 clock_t *outputsent     = (clock_t*)malloc(sizeof(clock_t)*ninfname);
 clock_t *memoryreleased = (clock_t*)malloc(sizeof(clock_t)*ninfname);

 unsigned int j;
 for(j = 0; j < ninfname; j++)
 {
  unsigned int prefixlength = strlen(prefix) + 1;
  char *fullname = malloc(prefixlength + strlen(infnamebatch[j]));
  memcpy(fullname, prefix, prefixlength);
  strcat(fullname, infnamebatch[j]);
  FILE* dicom = strcmp("-", infnamebatch[j]) ? fopen(fullname, "r") : stdin;

  if(dicom == NULL) 
  {
   fprintf(errfile, " ERROR 3: failed to open input file %s\n", infnamebatch[j]);
   if(errfile != stderr) fclose(errfile);
   return 3;
  }

  free(fullname);

  dcmbuff *buff; dcmbuff_loaddicom(&buff, dicom);
  if(dicom == stdin ? 0 : fclose(dicom))
   fprintf(errfile, " ERROR 4: failed to close input file %s; continuing\n", infnamebatch[j]);
  inputloaded[j] = clock();

  dcmelarr *metaarr; dcmelement_mkarr(&metaarr);
  tsmode mode;
  if(procfilemeta(metaarr, &mode, buff)) 
  {
   fprintf(errfile, " ERROR 5: failed to process file metadata elements\n");
   if(errfile != stderr) fclose(errfile);
   return 5;
  }

  dcmelarr *bodyarr; dcmelement_mkarr(&bodyarr);
  if(procfilebody(bodyarr, mode, buff))
  {
   fprintf(errfile, " ERROR 6: failed to process file body elements\n");
   if(errfile != stderr) fclose(errfile);
   return 6;
  }

  dcmbuff_del(buff);

  unsigned int i;
  if(omode.r)
  {
   for(i = 0; i < bodyarr->p; i++)
    if(bodyarr->els[i] != NULL)
     dcmtree_recursivehang(&bodyarr->els[i]);
   dcmtree_trim(bodyarr);
  }
  fileprocessed[j] = clock();

  omode.tag = infnamebatch[j];
  omode.current = j;
  if(dcmoutput_out(omode, metaarr, bodyarr))
  {
   fprintf(errfile, " ERROR 7: failed to write to file %s\n", log);
   if(errfile != stderr) fclose(errfile);
   return 7;
  }
  outputsent[j] = clock();

  if(dcmelement_delarr(metaarr)) fprintf(errfile, " ERROR 8: failed to free metadata array; continuing\n");
  if(dcmelement_delarr(bodyarr)) fprintf(errfile, " ERROR 9: failed to free body array; continuing\n");
  memoryreleased[j] = clock();
 }

 clock_t cputime = clock();
 time(&now);
 char cputimestr[20];

 for(j = 0; j < ninfname; j++)
 {
  formatcputime(cputimestr, inputloaded[j]);
  fprintf(errfile, " %s -- Input file loaded %s\n", cputimestr, infnamebatch[j]);
  formatcputime(cputimestr, fileprocessed[j]);
  fprintf(errfile, " %s -- File processed\n", cputimestr);
  formatcputime(cputimestr, outputsent[j]);
  fprintf(errfile, " %s -- Output written\n", cputimestr);
  formatcputime(cputimestr, memoryreleased[j]);
  fprintf(errfile, " %s -- Element arrays released\n", cputimestr);
 }

 formatcputime(cputimestr, cputime);
 fprintf(errfile, "%011ld  : %s   : Operations completed successfully\n", now, cputimestr);

 if(errfile == stderr ? 0 : fclose(errfile)) {perror("10: parsefile; continuing");}

 return 0;
}

#endif

int main(int argc, char** argv)
{
 return beginops(argc, argv);
}
