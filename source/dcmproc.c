#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCLUDESTDINT 0
#if INCLUDESTDINT == 1
#include <stdint.h>
#endif

#include "hougasargs.c"
#include "dcmtypes.c"
#include "dcmelement.c"
#include "dcmendian.c"
#include "dcmezbuff.c"
#include "dcmspecialtag.c"

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

 if(dcmbuff_get(&tmp, source, FIRSTPULL)) {perror("1:getelmeta"); return 1;}

 byte1 *buff = malloc(FIRSTPULL);
 if(buff == NULL) {perror("2:getelmeta"); return 2;}

 memcpy(buff,tmp,FIRSTPULL);
 dest->tag = *(byte4*)buff;
 dcmendian_handletag(&dest->tag, mode.e);

 if(dcmspecialtag_isnovr(dest->tag) || mode.v == v_implicit)
 {
  memcpy(dest->vr,"xx",2);
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
  if(dcmbuff_get(&tmp, source, SECONDPULL)) {perror("3:getelmeta"); return 3;}

  void *newmem = realloc(buff, FIRSTPULL + SECONDPULL);
  if(newmem == NULL) {perror("4:getelmeta"); return 4;}

  buff = (byte1*)newmem;
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
 if(dest == NULL || source == NULL) {perror("1:geteldata"); return 1;}

 if(dcmspecialtag_issq(dest->tag) || dest->tag == dcmspecialtag_ITEM) return 0;

 byte1 *tmp;
 if(dcmbuff_get(&tmp, source, dest->length)) {perror("2:geteldata"); return 2;}

 dest->data = malloc(dest->length);
 if(dest->data == NULL) {perror("3:geteldata"); return 3;}

 memcpy(dest->data, tmp, dest->length);

 return 0;
}

/*
 grab el from buff, process, place in arr
*/
int getputel(dcmel **el, dcmelarr *arr, dcmbuff *source, tsmode mode)
{
 if(el == NULL || arr == NULL || source == NULL || source->data == NULL) {perror("1:getputel"); return 1;}

 *el = (dcmel*)malloc(sizeof(dcmel));
 if(*el == NULL) {perror("2:getputel"); return 2;}

 if(getelmeta(*el, source, mode)) {perror("3:getputel"); return 3;}

 if(geteldata(*el, source)) {perror("4:getputel"); return 4;}

 if(dcmelement_addel(arr, *el)) {perror("5:getputel"); return 5;}

 return 0;
}

/*
 process the dicom file metadata into dcmels -> array
*/
int procfilemeta(dcmelarr **arr, tsmode **filemode, dcmbuff *source)
{
 if(arr == NULL || filemode == NULL) {perror("1:procfilemeta"); return 1;}

 dcmel *el;

 if(dcmelement_mkarr(arr)) {perror("2:procfilemeta"); return 2;}

 if(getputel(&el, *arr, source, FILEMETATS)) {perror("3:procfilemeta"); return 3;}

 byte4 datanumber;
 memcpy(&datanumber, el->data, sizeof(byte4));
 if(!dcmendian_SYSISLITTLE)
  datanumber = dcmendian_4flip(datanumber);
 int filemetastop = source->p + datanumber;
 *filemode = NULL;

 while(source->p < filemetastop) /* this will not work with dcmsmartbuff unless the first pull is good */
 {
  if(getputel(&el, *arr, source, FILEMETATS)) {perror("4:procfilemeta"); return 4;}

  if(el->tag == dcmspecialtag_TSUID)
   if(dcmspecialtag_tsdecode(filemode, el->data, el->length)) {perror("5:procfilemeta"); return 5;}
 }

 if(*filemode == NULL) {perror("6:procfilemeta"); return 6;}

 return 0;
}

/*
 process dicom file body into dcmels -> array
*/
int procfilebody(dcmelarr **arr, tsmode filemode, dcmbuff *source)
{
 if(arr == NULL) {perror("1:procfilebody"); return 1;}

 dcmel *el;

 if(dcmelement_mkarr(arr)) {perror("2:procfilebody"); return 2;}

 while(source->p < source->l) /* this will not work with dcmsmartbuff */
  if(getputel(&el, *arr, source, filemode)) {perror("3:procfilebody"); return 3;}

 return 0;
}

int dooutput(char *outfname, m_format format, dcmelarr *meta, dcmelarr *body)
{
 int i,j;
 FILE *outfile = strcmp("-",outfname) ? fopen(outfname, "w") : stdout;
 if(outfile == NULL) {perror("1:dooutput"); return 1;}

 switch(format)
 {
 case f_json:
  fprintf(outfile, "{\n");

  fprintf(outfile, " \"meta\": [\n");
  for(i = 0; i < meta->p; i++)
  {
   dcmel *el = meta->els[i];
  fprintf(outfile, "  {\n");
  fprintf(outfile, "   \"tag\": %d,\n", el->tag);
  fprintf(outfile, "   \"vr\": \"%c%c\",\n", el->vr[0], el->vr[1]);
  fprintf(outfile, "   \"length\": %d,\n", el->length);
  fprintf(outfile, "   \"value\": \"");
   for(j = 0; j < el->length; j++)
   {
    fprintf(outfile, "%02x", el->data[j]);
    if(j < el->length - 1)
     fprintf(outfile, " ");
    else
     fprintf(outfile,"\"\n");
   }
  fprintf(outfile, "  }");
   if(i < meta->p - 1)
    fprintf(outfile, ",\n");
   else
    fprintf(outfile, "\n");
  }
  fprintf(outfile, " ],\n");

  fprintf(outfile, " \"body\": [\n");
  for(i = 0; i < body->p; i++)
  {
   dcmel *el = body->els[i];
  fprintf(outfile, "  {\n");
  fprintf(outfile, "   \"tag\": %d,\n", el->tag);
  fprintf(outfile, "   \"vr\": \"%c%c\",\n", el->vr[0], el->vr[1]);
  fprintf(outfile, "   \"length\": %d,\n", el->length);
  fprintf(outfile, "   \"value\": \"");
   for(j = 0; j < el->length; j++)
   {
    fprintf(outfile, "%02x", el->data[j]);
    if(j < el->length - 1)
     fprintf(outfile, " ");
    else
     fprintf(outfile,"\"\n");
   }
  fprintf(outfile, "  }");
   if(i < body->p - 1)
    fprintf(outfile, ",\n");
   else
    fprintf(outfile, "\n");
  }
  fprintf(outfile, " ]\n");
 
  fprintf(outfile, "}");

 break;

 default:
  fprintf(outfile, "***METASTART***\n");

  for(i = 0; i < meta->p; i++)
  {
   dcmel *el = meta->els[i];
   fprintf(outfile, "%08x,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);
 
   for(j = 0; j < el->length; j++)
    fprintf(outfile, "%02x ", el->data[j]);
   fprintf(outfile, "\n");
  }

  fprintf(outfile, "***BODYSTART***\n");
 
  for(i = 0; i < body->p; i++)
  {
   dcmel *el = body->els[i];
   fprintf(outfile, "%08x,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);
 
   for(j = 0; j < el->length; j++)
    fprintf(outfile, "%02x ", el->data[j]);
   fprintf(outfile, "\n");
  }
 } 

 if(fclose(outfile)) {perror("2:dooutput"); return 2;}

 return 0;
}

int doflagstuff(void **pchart, int argc, char **argv)
{
 char *FLAG_HELP[] = {"\0","h","help",NULL};
 char *FLAG_VERSION[] = {"\0","v","version",NULL};
 char *FLAG_CSV[] = {"\0","c","csv",NULL};
 char *FLAG_FILE[] = {"\1","f","file","input",NULL};
 char *FLAG_JSON[] = {"\0","j","json",NULL};
 char *FLAG_OUTPUT[] = {"\1","o","output",NULL};
 char **VALIDFLAGS[] = {FLAG_HELP, FLAG_VERSION, FLAG_CSV, FLAG_FILE, FLAG_JSON, FLAG_OUTPUT,NULL};

 hougasargs_argproc(pchart, VALIDFLAGS, argc, argv);
 void *chart = *pchart;

 if(hougasargs_flagcount(chart, 0))
 {
  printf("-h, --help    : this\n");
  printf("-v, --version : version info (build date)\n");
  printf("-c, --csv     : output in CSV format (default)\n");
  printf("-f, --file    : file to process stdin is default\n");
  printf("    --input");
  printf("-j, --json    : output in JSON format\n");
  printf("-o, --output  : file to write to (kablam!) stdout is default\n");
  exit(0);
 }
 if(hougasargs_flagcount(chart, 1))
 {
  printf("Built on %s\n", __DATE__);
  exit(0);
 }
 if(hougasargs_flagvalue(chart, 3) == NULL)
 {
  fprintf(stderr,"Input file not specified; assuming stdin\n");
  hougasargs_flagvalue(chart, 3) = "-";
 }
 if(hougasargs_flagvalue(chart, 5) == NULL)
 {
  fprintf(stderr,"Output file not specified: assuming stdout\n");
  hougasargs_flagvalue(chart, 5) = "-";
 }

 return 0;
}

int parsefile(int argc, char **argv)
{
 void *chart;
 doflagstuff(&chart, argc, argv);
 char* infname = hougasargs_flagvalue(chart, 3);
 m_format format = hougasargs_flagcount(chart, 4) ? f_json : f_csv;
 char* outfname = hougasargs_flagvalue(chart, 5);

 FILE* dicom = strcmp("-",infname) ? fopen(infname, "r") : stdin;
 if(dicom == NULL) {perror("1:parsefile"); return 1;}

 dcmbuff *onebuff;
 dcmbuff_loaddicom(&onebuff, dicom);
 if(fclose(dicom)) {perror("2:parsefile; continuing");}

 dcmelarr *metaarr;
 tsmode *mode;
 procfilemeta(&metaarr, &mode, onebuff);

 dcmelarr *bodyarr;
 procfilebody(&bodyarr, *mode, onebuff);

 if(dooutput(outfname, format, metaarr, bodyarr)) {perror("3:parsefile"); return 3;}

 return 0;
}

int main(int argc, char** argv)
{
 return parsefile(argc, argv);
}
