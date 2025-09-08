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

const int FILEMETATS[] = {v_explicit,e_little};

/*
 From dicom standard 5.7.1
 mode [m_vr,m_endian]
 = 0: success
 = 1: failed first pull (8 bytes)
 = 2: failed to allocate memory
 = 3: failed second pull (+4 bytes)
*/
int getelmeta(dcmel *dest, dcmbuff *source, const int *mode)
{
 byte1 *tmp;
 const int FIRSTPULL = 8;
 if(dcmbuff_get(&tmp, source, FIRSTPULL)) {perror("1:getelmeta"); return 1;}

 byte1 *buff = malloc(FIRSTPULL);
 if(buff == NULL) {perror("2:getelmeta"); return 2;}

 memcpy(buff,tmp,FIRSTPULL);
 dest->tag = *(byte4*)buff;
 dcmendian_handletag(&dest->tag, mode[1]);

 if(dcmspecialtag_isnovr(dest->tag) || mode[0] == v_implicit)
  dest->length = ((byte4*)buff)[1];
 else if(dcmspecialtag_isshortvr(&buff[4]))
 {
  dest->vr[0] = buff[4]; dest->vr[1] = buff[5];
  dest->length = ((byte2*)buff)[3];
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
 }

 if(*dcmendian_SYSISLITTLE != mode[1])
  dcmendian_4flip(dest->length);

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
 byte1 *tmp;
 if(dcmbuff_get(&tmp, source, dest->length)) {perror("1:geteldata"); return 1;}

 dest->data = malloc(dest->length);
 if(dest->data == NULL) {perror("2:geteldata"); return 2;}

 memcpy(dest->data, tmp, dest->length);

 return 0;
}

int procfilemeta(dcmelarr **arr, dcmbuff *buff)
{
 if(dcmelement_mkarr(arr)) {perror("1:procfilemeta"); return 1;}

 dcmel *current;
 byte4 nexttag;

 do
 {
  current = malloc(sizeof(dcmel));
  if(current == NULL) {perror("2:procfilemeta"); return 2;}

  if(getelmeta(current, buff, FILEMETATS)) {perror("3:procfilemeta"); return 3;}

  if(geteldata(current, buff)) {perror("4:procfilemeta"); return 4;}

  if(dcmelement_addel(*arr, current)) {perror("5:procfilemeta"); return 5;}

  byte1 *tmp;
  if(dcmbuff_peek(&tmp, buff, sizeof(byte4))) {perror("6:procfilemeta"); return 6;}

  memcpy(&nexttag,tmp,sizeof(byte4)); 
  dcmendian_handletag(&nexttag, e_little); 
 } while(((nexttag)&0xFFFF0000) == 0x00020000);
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
 else if(mjhargsv(flagchart,4)==NULL)
 {
  printf("Tag(s) not specified\n");
  exit(2);
 }
 else
 {
  if(mjhargsv(flagchart,3)==NULL) mjhargsv(flagchart,3)="0";
 }

 return 0;
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

 procfilemeta(&arr, zero);

 int i,j;
 for(i = 0; i < arr->p; i++)
 {
  dcmel *el = arr->els[i];
  printf("%08x %c%c %d\n",el->tag, el->vr[0], el->vr[1], el->length);

  for(j = 0; j < el->length; j++)
   printf("%02x ", el->data[j]);
  printf("\n***\n");
 }
}

int run(int argc, char **argv)
{
 void* flagchart;
 char* validflags[] = {"h","help","v","version","f","file","s","start","t","tag","--"};

 mjhargsproc(&flagchart,validflags,argc,argv);
 flagcaveats(flagchart);

 FILE* dicom = fopen(mjhargsv(flagchart,2),"r");
 dcmbuff *zero;

 dcmbuff_loaddicom(&zero, dicom);

 dcmel el[2];
 int mode[] = {1,1};
 int i,j;

 for(i=0;i<2;i++)
 {
  getelmeta(&el[i],zero,mode);
  geteldata(&el[i],zero);

  printf("%08x %c%c %d\n",el[i].tag,el[i].vr[0],el[i].vr[1],el[i].length);
/*  printf("%x %x\n",el[i].buffnum,ftell(dicom));
*/
  for(j=0;j<el[i].length;j++)
   printf("%02x ",el[i].data[j]);
  printf("\n***\n");
 }

 fclose(dicom);
 return 0;
}

int main(int argc, char** argv)
{
 filemetatest(argc, argv);
 return 0;
}
