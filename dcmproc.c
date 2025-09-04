#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hougasargs.c"

#define INCLUDESTDINT 0
#if INCLUDESTDINT == 1
 #include <stdint.h>
#endif

#define BO0 0x000000FF
#define BO1 0x0000FF00
#define BO2 0x00FF0000
#define BO3 0xFF000000
#define NSQS 1324
#define PSQS "sqs"
#define LSQS 18
#define byte(n) (0xFF<<n)
/*
 300a00b2 => 0a30b200
*/
#define mjhflip(a) (((a&0x00ff0000)<<8)+((a&0xff000000)>>8)+((a&0x000000ff)<<8)+((a&0x0000ff00)>>8))
#define mjhgtag(data) (((long*)data)[0])
#define mjhgend(data) (((long*)data)[1])
#define mjhgpload(data) ((char*)&((long*)data)[2])
#define ptrchg(p,t,n) (((t*)(p))[n])

/***
 use stdint if available
***/
#ifdef _STDINT_H
 typedef uint8_t byte1;
 typedef uint16_t byte2;
 typedef uint32_t byte4;
#else
 typedef unsigned char byte1;
 typedef unsigned short byte2;
 typedef unsigned int byte4;
#endif

typedef enum
{
 e_big,
 e_little
} m_endian;

typedef enum
{
 v_implicit,
 v_explicit
} m_vr;

const unsigned int DCMBUFFLEN = 0x40000000;

/***
 assuming byte 0 is on the left
 lendain 1 = 0x01000000
 bendian 1 = 0x00000001
 if(*SYSLENDIAN) little endian stuff; else big endian stuff; 
***/
const unsigned int ENDIAN1 = 1;
const byte1 *SYSLENDIAN = (byte1*)&ENDIAN1;

/*
 moved to dcmsmartbuff.c
 not ready to finalize yet
*/
typedef struct
{
 unsigned int num;
 byte1 *raw;
 unsigned long long len;
 unsigned long long pos;
} dcmbuff;

int dcmbuffdel(dcmbuff *buff)
{
 free(buff->raw);
 free(buff);
 return 0;
}

/***
 bound [start position of element in file, end position of element in file]
 tag [group, element] Data Element Tag see part 5 section 7.1.1
 vr Value Representation see part 5 section 7.1.1
 length Value Length see part 5 section 7.1.1
 data Value Field see part 5 section 7.1.1
 datastop should be equal to length unless length is 0xFFFFFFFF
***/
typedef struct
{
 unsigned long long buffnum;
 unsigned long long pos;
 byte2 tag[2];
 byte1 vr[2];
 unsigned int length;
 byte1* data;
 unsigned long long datastop;
} dcmel;

/***
 free(&dcmel) is bad
***/
int dcmeldel(dcmel *element)
{
 free(element->data);
 free(element);
 return 0;
}

/***
 From DICOM standard part 5 section 7.1.2
 these vrs imply a 2 byte length length with explicit vrs
***/
int isshortvr(char* vr)
{
 const char* VRSHORTS[] = {"AE","AS","AT","CS","DA","DS","DT","FL","FD","IS","LO","LT","PN","SH","SL","SS","ST","TM","UI","UL","US"};
 const unsigned int NVRSHORT = 21;
 int i;
 for(i=0;i<NVRSHORT && strncmp(VRSHORTS[i],vr,2);i++);
 return i<NVRSHORT;
}

/***
 From DICOM standard part 5 section 7.5
 these two tags end elements of undefined length (0xFFFFFFFF)
 {item delimitation group, item delimitation element, sequence delimitation group, sequence delimitation element}
***/
const short NUMDELIMITATION = 2;
const short DELIMITATION[] = {0xFFFE,0xE00D,0xFFFE,0xE0DD};

/***
 From DICOM standard part 5 section 7.5
 these tags do not have vrs
***/
int isnovr(byte2 *tag)
{
 const unsigned int NNOVRS = 3;
 const unsigned int NOVRS[] = {0xFFFEE00D,0xFFFEE0DD,0xFFFEE000};
 int i;
 for(i=0;i<NNOVRS && !(tag[0]==(NOVRS[i]&0xFFFF0000)>>16 && tag[1]==NOVRS[i]&0x0000FFFF);i++);
 return i<NNOVRS;
}

int issq(byte4 tag)
{
 static unsigned int *sqsl = NULL;
 static unsigned int *sqsh = NULL;
 if(sqsl == NULL || sqsh == NULL)
 {
  sqsl = malloc(sizeof(int)*NSQS);
  sqsh = malloc(sizeof(int)*NSQS);
  char buff[LSQS+1];
  char *end;
  FILE *fsqs = fopen(PSQS,"r");
  unsigned int i;

  fgets(buff,LSQS,fsqs);
  for(i=0; i<NSQS && !feof(fsqs); i++);
  {
   sqsl[i] = (unsigned int)strtoul(buff, &end, 16);
   sqsh[i] = (unsigned int)strtoul(&buff[9], &end, 16);
   if(sqsl[i] == 0 || sqsh[i] == 0) return -1;
   fgets(buff,LSQS,fsqs);
  }

  fclose(fsqs);
 }

 int low = 0, high = NSQS-1, mid;
 do                                                                                                                       
 {
  mid = (high - low)/2;
  if(tag >= sqsl[mid] && tag <= sqsh[mid]) return 1;
  if(tag > sqsl[mid]) low = mid;
  else high = mid;
 } while(low <= high);
 
 return 0;
}

/***
 little endian <-> big endian
***/
int endianswap(byte1* toswap, unsigned int size)
{
 int i;
 for(i=0;i<size/2;i++)
 {
  toswap[i]^=toswap[size-1-i];
  toswap[size-1-i]^=toswap[i];
  toswap[i]^=toswap[size-1-i];
 }
}

int firstbuff(dcmbuff **zero)
{
 *zero=malloc(sizeof(dcmbuff));
 (*zero)->num=0;
 (*zero)->raw=malloc(DCMBUFFLEN);
 (*zero)->len=DCMBUFFLEN;
 (*zero)->pos=132;
 return 0;
}

/***
return 0: buffer pulled normally
return 1: EOF encountered
return 2: other error
***/
int pullbuff(dcmbuff **new, FILE *dicom, unsigned int topull, dcmbuff *old)
{
 unsigned int leftover = old->len - old->pos;
 unsigned long long read;

 *new = malloc(sizeof(dcmbuff));

 (*new)->num = old->num+1;
 (*new)->pos = 0;
 (*new)->len = leftover+topull;
 (*new)->raw = (byte1*)malloc((*new)->len);

 read = fread(&(*new)->raw[leftover], 1, topull+leftover, dicom);
 (*new)->len = read+leftover;

 dcmbuffdel(old);

 if(feof(dicom)) return 1;
 if(ferror(dicom)) return 2;

 return 0;
}

/***
 return 0: read in full buffer, DICM found @ 0x0100
 return 1: read in partial buffer, DICM found @ 0x100
 return 2: DICM not found @ 0x0100
 return 3: other error;
***/
int initdicom(dcmbuff** zero,FILE* dicom)
{
 unsigned long long read;
 firstbuff(zero);

 if((read = fread((*zero)->raw,DCMBUFFLEN,1,dicom)) > 134) return 2;
 if(strncmp(&(*zero)->raw[128],"DICM",4)) return 2;
 if(feof(dicom)) return 1;
 if(ferror(dicom)) return 3;

 return 0;
}

/***
 From DICOM standard part 5 section 7.1
 source must have 8 bytes
 mode[0]: 0 means implicit vr
 mode[1]: 0 means bigendian
 return 0: data parsed into dest
 return 1: buffer end encountered
 return 2: other error
***/
int getelmeta(dcmel *dest, dcmbuff *source, int *mode)
{
 if(source->pos - source->len < 8) return 1;
 unsigned long long extra = source->len - DCMBUFFLEN;
 
 byte1* buff = &source->raw[source->pos];

 dest->buffnum = (source->len - source->pos > DCMBUFFLEN) ? (source->num - 1) : (source->num);
 dest->pos = (source->num == dest->buffnum) ? (DCMBUFFLEN - extra + source->pos) : (source->pos - extra);
 source->pos += 8;

 *(byte4*)dest->tag=*(byte4*)buff;

 if(*SYSLENDIAN!=mode[1])
 {
  dest->tag[0] = dest->tag[0]&BO0<<8 + dest->tag[0]&BO1>>8;
  dest->tag[1] = dest->tag[1]&BO0<<8 + dest->tag[1]&BO1>>8;
 }

 if(isnovr(dest->tag) || !mode[0])
  dest->length = ((byte4*)buff)[1];
 else if(isshortvr(&buff[4]))
 {
  memcpy(dest->vr,&buff[4],2);
  dest->length=((byte2*)buff)[3];
 }
 else
 {
  if(source->len - source->pos < 4)
  {
   source->pos -= 8;
   return 1;
  }
  source->pos += 4;
  memcpy(dest->vr, &buff[4], 2);
  dest->length=((byte4*)buff)[2];
  printf("%c%c %d ***\n",dest->vr[0],dest->vr[1],((int*)buff)[2]);
 }

 if(*SYSLENDIAN!=mode[1])
  endianswap((byte1*)&dest->length, isshortvr(dest->vr) ? 2 : 4);

 return 0;
}

/***
 return 0: buffer copied to data OR length = 0xFFFFFFFF and no copy
 return 1: length != 0xFFFFFFFF and end of buffer encountered
***/
int geteldata(dcmel *dest, dcmbuff *source)
{
 if(dest->length != 0xFFFFFFFF)
 {
  dest->data = (byte1*)malloc(dest->length);
  if(dest->length > source->len - source->pos)
   return 1;
  else
  {
   memcpy(dest->data, &source->raw[source->pos], dest->length);
   source->pos += dest->length;
  }
 }
 else
  dest->data = malloc(0);
 
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

int run(int argc,char** argv)
{
 void* flagchart;
 char* validflags[] = {"h","help","v","version","f","file","s","start","t","tag","--"};

 mjhargsproc(&flagchart,validflags,argc,argv);
 flagcaveats(flagchart);

 FILE* dicom = fopen(mjhargsv(flagchart,2),"r");
 dcmbuff* zero;
 int initcode = initdicom(&zero,dicom);

 dcmel el[2];
 int mode[] = {1,1}; 
 
 int i,j;

 for(i=0;i<2;i++)
 {
  getelmeta(&el[i],zero,mode);
  geteldata(&el[i],zero);

  printf("%04x %04x %c%c %d\n",el[i].tag[0],el[i].tag[1],el[i].vr[0],el[i].vr[1],el[i].length);
  printf("%x %x %x %d\n",el[i].buffnum,zero->pos,ftell(dicom),initcode);

  for(j=0;j<el[i].length;j++) printf("%02x ",el[i].data[j]);
  printf("\n***\n");
 }

 fclose(dicom);
 return 0;
}

int main(int argc, char** argv)
{
 run(argc,argv);

 return 0;
}
