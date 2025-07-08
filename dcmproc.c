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
 hope char is 1 byte.
***/
#if INCLUDESTDINT == 1
 typedef uint8_t byte1;
 typedef uint16_t byte2;
 typedef uint32_t byte4;
#else
 typedef unsigned char byte1;
 typedef unsigned short byte2;
 typedef unsigned int byte4;
#endif

const unsigned int DCMBUFFLEN = 0x40000000;

/***
 assuming byte 0 is on the left
 lendain 1 = 0x01000000
 bendian 1 = 0x00000001
***/
const unsigned int ENDIAN1 = 1;
const byte1* SYSLENDIAN = (byte1*)&ENDIAN1;

/***
 a buffer to pull file data into
 maintains basic metadata
***/
typedef struct
{
 unsigned int num;
 byte1* raw;
 unsigned long long len;
 unsigned long long pos;
}dcmbuff;

int dcmbuffdel(dcmbuff* buff)
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
}dcmel;

/***
 free(&dcmel) is bad
***/
int dcmeldel(dcmel* element)
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
 for(i=0;i<NVRSHORT && !strncmp(VRSHORTS[i],vr,2);i++);
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
int isnovr(byte2* tag)
{
 const unsigned int NNOVRS = 3;
 const unsigned int NOVRS[] = {0xFFFEE00D,0xFFFEE0DD,0xFFFEE000};
 int i;
 for(i=0;i<NNOVRS && !(tag[0]==(NOVRS[i]&0xFFFF0000)>>16 && tag[1]==NOVRS[i]&0x0000FFFF);i++);
 return i<NNOVRS;
}

/***
 little endian <-> big endian
***/
int endianswap(byte1* toswap,unsigned int size)
{
 int i;
 for(i=0;i<size/2;i++)
 {
  toswap[i]^=toswap[size-1-i];
  toswap[size-1-i]^=toswap[i];
  toswap[i]^=toswap[size-1-i];
 }
}

int firstbuff(dcmbuff** zero)
{
 *zero=malloc(sizeof(dcmbuff));
 (*zero)->num=0;
 (*zero)->raw=malloc(DCMBUFFLEN);
 (*zero)->len=DCMBUFFLEN;
 (*zero)->pos=132;
 return 0;
}

int pullbuff(dcmbuff** new,FILE* dicom,unsigned int topull,dcmbuff* old)
{
 unsigned int leftover = old->len - old->pos;
 unsigned long long read;

 *new = (dcmbuff*)malloc(sizeof(dcmbuff));

 (*new)->num = old->num+1;
 (*new)->pos = 0;
 (*new)->len = leftover+topull;
 (*new)->raw = (byte1*)malloc(sizeof(byte1)*(*new)->len);

 read = fread(&(*new)->raw[leftover],1,sizeof(byte1)*(topull+leftover),dicom);
 (*new)->len = read+leftover;

 dcmbuffdel(old);

 if(feof(dicom)) return 1; if(ferror(dicom)) return 2;
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
 if(feof(dicom)) return 1; if(ferror(dicom)) return 3;
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
int getelmeta(dcmel* dest, dcmbuff* source, int* mode)
{
 if(source->pos-source->len < 8) return 1;
 unsigned long long extra = source->len - DCMBUFFLEN;
 
 byte1* buff = &source->raw[source->pos];

 dest->buffnum = source->len - source->pos > DCMBUFFLEN ? source->num - 1 : source->num;
 dest->pos = source->num==dest->buffnum ? DCMBUFFLEN - extra + source->pos : source->pos - extra;
 source->pos += 8;

 *(byte4*)dest->tag=*(byte4*)buff;
 if(*SYSLENDIAN!=mode[1])
 {
  dest->tag[0] = dest->tag[0]&BO0<<8 + dest->tag[0]&BO1>>8;
  dest->tag[1] = dest->tag[1]&BO0<<8 + dest->tag[1]&BO1>>8;
 }

 if(isnovr(dest->tag) || !mode[0])
  dest->length=((byte4*)buff)[1];
 else if(isshortvr(&buff[4]))
 {
  memcpy(dest->vr,&buff[4],2);
  dest->length=((byte2*)buff)[3];
 }
 else
 {
  if(source->pos-source->len < 4) {source->pos -= 8; return 1;}
  source->pos += 4;
  memcpy(dest->vr,&buff[4],2);
  dest->length=((byte4*)buff)[2];
 }

 if(*SYSLENDIAN!=mode[1])
  endianswap((byte1*)&dest->length,isshortvr(dest->vr) ? 2 : 4);

 return 0;
}

int geteldata(dcmel* dest, byte1* source, unsigned int bytesleft, int* mode)
{
 return 0;
}

int flagcaveats(void* flagchart)
{
 if(mjhargsc(flagchart,0))
 {
  printf("-h, --help: this\n");
  printf("-v --version: version info\n");
  printf("-f --file: file to search\n");
  printf("-t, --tag: specify tag(s) to search for\n");
  printf("-s, --start: starting position in hex\n");
  exit(1);
 }
 else if(mjhargsc(flagchart,1))
 {
  printf("Built on __DATE__\n");
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
 initdicom(&zero,dicom);

 dcmel this;
 int mode[] = {1,1}; 
 
 getelmeta(&this,zero,mode);

 printf("%04x %04x %c%c %d\n",this.tag[0],this.tag[1],this.vr[0],this.vr[1],this.length);
 printf("%x %x %x %d\n",this.buffnum,zero->pos,ftell(dicom),*SYSLENDIAN);

 fclose(dicom);
 return 0;
}

int main(int argc, char** argv)
{
 run(argc,argv);

 return 0;
}
