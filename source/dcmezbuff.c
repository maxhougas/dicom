/*
 dcmezbuff.c
 structure and methods for implement the easier 1file-1suck method
 dcm file will be assumed (for now) to be < 2GB (addressable by signed int)
 check method should be implmented
 this will assume char is 8 bits = 1 byte
 this will assume int is 32 bits = 4 bytes

 anything replacing this (dcmsmartbuff.c) should implement
 dcmbuff
 dcmbuff_del
 dcmbuff_get
 dcmbuff_peek
 dcmbuff_loaddicom
*/

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _DCMTYPES
#include "dcmtypes.c"
#endif

#define _DCMBUFF 1
#define _DCMEZBUFF 1
/*
 this should NEVER be less than DICOMHEADERL + 4
*/
#define dcmezbuff_DICOMSIZEMAX (0x01 << 30)
#define dcmezbuff_DICOMHEADERL 128
#define dcmezbuff_DICOMFOURCC "DICM"

typedef struct
{
 unsigned int l;
 unsigned int p;
 byte1 *data;
} dcmbuff;

void dcmbuff_del(dcmbuff *todel)
{
 free(todel->data);
 free(todel);
}

/*
 = 0: success
 = 1: null parameter(s)
 = 2: insufficient data
*/
int dcmbuff_get(byte1 **current, dcmbuff *buff, int numchars)
{
 if(current == NULL || buff == NULL || buff->data == NULL) {perror("1:dcmbuff_get"); return 1;}

 if(buff->l-buff->p < numchars || numchars < 0) {perror("2:dcmbuff_get"); return 2;}

 *current = &buff->data[buff->p];
 buff->p += numchars;

 return 0;
}

int dcmbuff_peek(byte1 **current, dcmbuff *buff, int numchars)
{
 if(*current == NULL || buff == NULL || buff->data == NULL) {perror("1:dcmbuff_peek"); return 1;}
 
 if(buff->l-buff->p < numchars || numchars < 0) {perror("2:dcmbuff_peek"); return 2;}

 *current = &buff->data[buff->p];

 return 0;
}

int dcmezbuff_filetoobig(FILE *dicom)
{
 if(fseek(dicom, 0, SEEK_END)) {perror("1:dcmbuff_filetoobig"); return 1;}
 unsigned long int size = ftell(dicom);
 if(size == -1L) {perror("2:dcmbuff_filetoobig"); return 2;}
 if(size > dcmezbuff_DICOMSIZEMAX) {perror("3:dcmbuff_filetoobig"); return 3;}

 return 0;
}

/*
 = -1: encountered eof during file read (should never happen)
 =  0: success
 =  1: null parameter(s)
 =  2: filetoobig failed
 =  3: failed to determine file size (should never happen)
 =  4: failed to allocate memory
 =  5: unspecified file read error
 =  6: fourcc check failed
*/
int dcmbuff_loaddicom(dcmbuff **pbuff, FILE *dicom)
{
 if(pbuff == NULL || dicom == NULL) return perror("1:dcmbuff_loaddicom"), 1;

 byte1 *data;
 unsigned int nread;

 if(dicom == stdin)
 {
  data = (byte1*)malloc(dcmezbuff_DICOMSIZEMAX);
  if(data == NULL) return perror("2:dcmbuff_loaddicom"), 2;

  nread = fread(data, 1, dcmezbuff_DICOMSIZEMAX, stdin);
  if(ferror(stdin)) return perror("3:dcmbuff_loaddicom"), 3;

  void* old = data;
  if((data = realloc(data, nread)) == NULL) return perror("4:dcmbuff_loaddicom"), 4;
  free(old);
 }
 else
 {
  if(dcmezbuff_filetoobig(dicom)) return perror("2:dcmbuff_loaddicom"), 2;

  long int size = ftell(dicom);
  if(size == -1L) return perror("3:dcmbuff_loaddicom"), 3;

  rewind(dicom);
  data = (byte1*)malloc(size);
  if(data == NULL) return perror("4:dcmbuff_loaddicom"), 4;

  nread = fread(data, 1, size, dicom);
  if(ferror(dicom)) return perror("5:dcmbuff_loaddicom"), 5;
 }

 *pbuff = (dcmbuff*)malloc(sizeof(dcmbuff));
 if(*pbuff == NULL) return perror("6:dcmbuff_loaddicom"), 6;

 (*pbuff)->data = data;
 (*pbuff)->p = dcmezbuff_DICOMHEADERL;
 (*pbuff)->l = nread;
 byte1 *tocheck;

 if
 (
  nread < dcmezbuff_DICOMHEADERL ||
  dcmbuff_get(&tocheck, *pbuff, strlen(dcmezbuff_DICOMFOURCC)) ||
  strncmp(tocheck, dcmezbuff_DICOMFOURCC, strlen(dcmezbuff_DICOMFOURCC))
 ) return perror("7:dcmbuff_loaddicom"), 7;

 return 0;
}
