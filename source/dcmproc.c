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

const tsmode FILEMETATS = {v_explicit,e_little};

/*
 From dicom standard 5.7.1
 mode [m_vr,m_endian]
 = 0: success
 = 1: failed first pull (8 bytes)
 = 2: failed to allocate memory
 = 3: failed second pull (+4 bytes)
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
 = 0: success
 = 1: failed dcmbuff_get
 = 2: failed to allocate memory

 add code to handle sequence/item
*/
int geteldata(dcmel *dest, dcmbuff *source)
{
 if(dest == NULL || source == NULL) {perror("2:geteldata"); return 1;}

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

int flagcaveats(void* flagchart)
{
 if(mjhargsc(flagchart,0))
 {
  printf("-h, --help: this\n");
  printf("-v, --version: version info\n");
  printf("-f, --file: file to search\n");
  printf("-t, --tag: specify tag(s) to search for\n");
  printf("-s, --start: starting position in hex\n");
  exit(1);
 }
 else if(mjhargsc(flagchart,1))
 {
  printf("Built on %s\n",__DATE__);
  exit(1);
 }
 else if(mjhargsv(flagchart,2)==NULL)
 {
  printf("File not specified\n");
  exit(2);
 }
 else
 {
  if(mjhargsv(flagchart,3)==NULL) mjhargsv(flagchart,3)="0";
 }

 return 0;
}

int filebodytest(int argc, char **argv)
{
 void* flagchart;
 char* validflags[] = {"h","help","v","version","f","file","s","start","t","tag","--"};

 mjhargsproc(&flagchart,validflags,argc,argv);
 flagcaveats(flagchart);

 FILE* dicom = fopen(mjhargsv(flagchart,2),"r");
 dcmbuff *zero;

 dcmbuff_loaddicom(&zero, dicom);
 dcmelarr *metaarr;
 tsmode *mode;
 procfilemeta(&metaarr, &mode, zero);

 dcmelarr *bodyarr;
 procfilebody(&bodyarr, *mode, zero);

 int i,j;
 for(i = 0; i < metaarr->p; i++)
 {
  dcmel *el = metaarr->els[i];
  printf("%08x,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);

  for(j = 0; j < el->length; j++)
   printf("%02x ", el->data[j]);
  printf("\n");
 }

 printf("***BODYSTART***\n");

 for(i = 0; i < bodyarr->p; i++)
 {
  dcmel *el = bodyarr->els[i];
  printf("%08x,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);

  for(j = 0; j < el->length; j++)
   printf("%02x ", el->data[j]);
  printf("\n");
 }
}

int filemetatest(int argc, char **argv)
{
 void* flagchart;
 char* validflags[] = {"h","help","v","version","f","file","s","start","t","tag","--"};

 mjhargsproc(&flagchart,validflags,argc,argv);
 flagcaveats(flagchart);

 FILE* dicom = fopen(mjhargsv(flagchart,2),"r");
 dcmbuff *zero;

 dcmbuff_loaddicom(&zero, dicom);
 dcmelarr *arr;
 tsmode *mode;

 procfilemeta(&arr, &mode, zero);

 int i,j;
 for(i = 0; i < arr->p; i++)
 {
  dcmel *el = arr->els[i];
  printf("%08x,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);

  for(j = 0; j < el->length; j++)
   printf("%02x ", el->data[j]);
  printf("\n");
 }

 printf("\n\n %d %d\n", mode->v, mode->e);

 return 0;
}

int main(int argc, char** argv)
{
 filebodytest(argc, argv);
 return 0;
}
