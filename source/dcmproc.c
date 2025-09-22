#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INCLUDESTDINT 0
#if INCLUDESTDINT == 1
#include <stdint.h>
#endif

#include "hougasargs.c"
#include "dcmtypes.c"
#include "dcmelement.c"
#include "dcmendian.c"
#include "dcmezbuff.c"
#include "dcmoutput.c"
#include "dcmspecialtag.c"
#include "dcmtree.c"

#define FNAMEL 255

const tsmode FILEMETATS = {v_explicit,e_little};

/*
 From dicom standard 5.7.1
 read and parse element metadata
*/
int getelmeta(dcmel *dest, dcmbuff *source, const tsmode mode)
{
 byte1 *tmp;
 const int FIRSTPULL = 8;

 if(dcmbuff_get(&tmp, source, FIRSTPULL)) return perror("1:getelmeta"), 1;

 byte1 *buff = malloc(FIRSTPULL);
 if(buff == NULL) return perror("2:getelmeta"), 2;

 memcpy(buff,tmp,FIRSTPULL);
 dest->tag = *(byte4*)buff;
 dcmendian_handletag(&dest->tag, mode.e);

 if(dcmspecialtag_isnovr(dest->tag) || mode.v == v_implicit)
 {
  memset(dest->vr,'x',2);
  dest->length = ((byte4*)buff)[1];
  dest->metalength = 8;
 }
 else if(dcmspecialtag_isshortvr(&buff[4]))
 {
  dest->vr[0] = buff[4]; dest->vr[1] = buff[5];
  dest->length = ((byte2*)buff)[3];
  dest->metalength = 8;
 }
 else /*explicit vr, not short*/
 {
  const int SECONDPULL = 4;
  if(dcmbuff_get(&tmp, source, SECONDPULL)) return perror("3:getelmeta"), 3;

  if((buff = realloc(buff, FIRSTPULL + SECONDPULL)) == NULL) return perror("4:getelmeta"), 4;

  memcpy(&buff[FIRSTPULL], tmp, SECONDPULL);
  dest->vr[0] = buff[4]; dest->vr[1] = buff[5];
  dest->length=((byte4*)buff)[2];
  dest->metalength = 12;
 }

 if(*dcmendian_SYSISLITTLE != mode.e)
  dest->length = dcmendian_4flip(dest->length);

 dest->rawmeta = buff;

 return 0;
}

/*
 copy element data from buffer
*/
int geteldata(dcmel *dest, dcmbuff *source)
{
 if(dest == NULL || source == NULL) return perror("1:geteldata"), 1;

 if(dcmspecialtag_ischildable(dest))
 {
  dest->effectivelength = 0;
  return 0;
 }
 else
  dest->effectivelength = dest->length;

 byte1 *tmp;
 if(dcmbuff_get(&tmp, source, dest->length)) return perror("2:geteldata"), 2;

 dest->data = malloc(dest->length);
 if(dest->data == NULL) perror("3:geteldata"), 3;

 memcpy(dest->data, tmp, dest->length);

 return 0;
}

/*
 grab el from source, process, place in arr
*/
int getputel(dcmelarr *arr, dcmbuff *source, tsmode mode)
{
 if(arr == NULL || source == NULL || source->data == NULL) return perror("1:getputel"), 1;

 dcmel *el = (dcmel*)malloc(sizeof(dcmel));
 if(el == NULL) return perror("2:getputel"), 2;

 el->nchildren = 0;
 if(getelmeta(el, source, mode)) return perror("3:getputel"), 3;

 if(geteldata(el, source)) return perror("4:getputel"), 4;

 if(dcmelement_addel(arr, el)) return perror("5:getputel"), 5;

 return 0;
}

/*
 process the dicom file metadata into dcmels -> array
*/
int procfilemeta(dcmelarr *arr, tsmode *filemode, dcmbuff *source)
{
 if(arr == NULL || filemode == NULL) return perror("1:procfilemeta"), 1;

 if(getputel(arr, source, FILEMETATS)) return perror("2:procfilemeta"), 2;

 byte4 datanumber;
 memcpy(&datanumber, (*arr->els)->data, sizeof(byte4));
 if(!dcmendian_SYSISLITTLE)
  datanumber = dcmendian_4flip(datanumber);
 int filemetastop = source->p + datanumber;


 while(source->p < filemetastop) /* this will not work with dcmsmartbuff unless the first pull is good */
 {
  if(getputel(arr, source, FILEMETATS)) return perror("3:procfilemeta"), 3;

  if(arr->els[arr->p-1]->tag == dcmspecialtag_TSUID)
   if(dcmspecialtag_tsdecode(filemode, arr->els[arr->p-1]->data, arr->els[arr->p-1]->length)) return perror("4:procfilemeta"), 4;
 }

 return 0;
}

/*
 process dicom file body into dcmels -> array
*/
int procfilebody(dcmelarr *arr, tsmode filemode, dcmbuff *source)
{
 if(arr == NULL) return perror("1:procfilebody"), 1;

 while(source->p < source->l) /* this will not work with dcmsmartbuff */
  if(getputel(arr, source, filemode)) return perror("2:procfilebody"), 2;

 return 0;
}

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

void formatcputime(char *str, clock_t cputime)
{
 unsigned int cpusec = cputime / CLOCKS_PER_SEC;
 unsigned int subsec = cputime % CLOCKS_PER_SEC;
                     /*0123456789012345678*/
 char subsecstr[18] = "                 \0";
 sprintf(subsecstr,"%-16lu",subsec + CLOCKS_PER_SEC);
 subsecstr[strlen(subsecstr)] = ' ';
 sprintf(str,"%03u.%s", cpusec, &subsecstr[1]);
}

void doflagstuff(hougasargs_flagchart *chart, int argc, char **argv)
{
 char *FLAG_HELP[] = {"\0","h","help",NULL};
 char *FLAG_VERSION[] = {"\0","v","version",NULL};
 char *FLAG_CSV[] = {"\0","c","csv","CSV",NULL};
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
/* 03 */ FLAG_FILE,
/* 04 */ FLAG_JSON,
/* 05 */ FLAG_LOG,
/* 06 */ FLAG_OUTPUT,
/* 07 */ FLAG_PREFIX,
/* 08 */ FLAG_RECURSE,
/* 09 */ FLAG_YAML,
 NULL
 };

 hougasargs_argproc(chart, VALIDFLAGS, argc, argv);

 if(chart->flagc[0])
 {
  printf("-h, --help    : this\n");
  printf("-v, --version : version info (build date)\n");
  printf("-c, --csv     : output in CSV format (default)\n");
  printf("    --CSV\n");
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
  printf("-y, --yaml    : output in YAML format\n");
  printf("    --YAML\n");
  exit(0);
 }
 if(chart->flagc[1])
 {
  printf("Built on %s\n", __DATE__);
  exit(0);
 }
 if(chart->flagc[2] && chart->flagc[8])
 {
  printf("Recursive mode not supported for CSV output.\n");
  exit(1);
 }
 if(chart->flagv[3] == NULL)
 {
  fprintf(stderr,"Input file not specified; assuming stdin\n");
  chart->flagv[3] = "-";
 }
 if(chart->flagv[5] == NULL)
 {
  fprintf(stderr,"Log file not specified; logging to stderr\n");
  chart->flagv[5] = "-";
 }
 if(chart->flagv[6] == NULL)
 {
  fprintf(stderr,"Output file not specified: assuming stdout\n");
  chart->flagv[6] = "-";
 }
 if(chart->flagv[7] == NULL)
 {
  chart->flagv[7] = "";
 }
}

int parsefile(int argc, char **argv)
{
 time_t now; time(&now);

 hougasargs_flagchart chart;
 doflagstuff(&chart, argc, argv);

 char* errfname = chart.flagv[5];
 FILE* errfile = strcmp("-",errfname) ? fopen(errfname, "a") : stderr;
 if(errfile == NULL) return perror("1:parsefile"), 1;

 fprintf(errfile,"%011ld  : ", now);
 struct tm *snow = gmtime(&now);
 int month = snow->tm_mon + 1;
 int year = snow->tm_year + 1900;
 fprintf(errfile,"%04d_%02d_%02d %02d:%02d:%02d Z  : Log file opened\n", year, month, snow->tm_mday, snow->tm_hour, snow->tm_min, snow->tm_sec);

 unsigned int infnamelength = strlen(chart.flagv[3]);
 char* infname = (char*)malloc(infnamelength+1);
 strcpy(infname, chart.flagv[3]);
 char **infnamebatch;
 unsigned int ninfname;
 tokenize(&infnamebatch, &ninfname, infname);
 m_format format = chart.flagc[2] ? f_csv  :
                   chart.flagc[4] ? f_json :
                   chart.flagc[9] ? f_yaml :
                                    f_csv  ;
 FILE *outfile = strcmp(chart.flagv[6], "-") ? fopen(chart.flagv[6], "w") : stdout;
 if(outfile == NULL)
 {
  fprintf(errfile, " ERROR 2: failed to open output file %s\n",  chart.flagv[6]);
  if(errfile != stderr) fclose(errfile);
  return 2;
 }
 outmode omode =
 {
  format,
  chart.flagc[8] ? 1 : 0,
  outfile,
  "",
  0,
  ninfname - 1
 };

 clock_t *inputloaded    = (clock_t*)malloc(sizeof(clock_t)*ninfname);
 clock_t *fileprocessed  = (clock_t*)malloc(sizeof(clock_t)*ninfname);
 clock_t *outputsent     = (clock_t*)malloc(sizeof(clock_t)*ninfname);
 clock_t *memoryreleased = (clock_t*)malloc(sizeof(clock_t)*ninfname);

 unsigned int j;
 for(j = 0; j < ninfname; j++)
 {
  unsigned int prefixlength = strlen(chart.flagv[7]) + 1;
  char *fullname = (char*)malloc(prefixlength + strlen(infnamebatch[j]));
  memcpy(fullname, chart.flagv[7], prefixlength);
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
   for(i = 0; i < bodyarr->p; i++)
    if(bodyarr->els[i] != NULL)
     dcmtree_recursivehang(&bodyarr->els[i]);
  fileprocessed[j] = clock();

  omode.tag = infnamebatch[j];
  omode.current = j;
  if(dcmoutput_out(omode, metaarr, bodyarr))
  {
   fprintf(errfile, " ERROR 7: failed to write to file %s\n", omode.outfile);
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

int main(int argc, char** argv)
{
 return parsefile(argc, argv);
}
