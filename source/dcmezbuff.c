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

#ifndef DCMTYPES
#include "dcmtypes.c"
#endif

#define DCMBUFF 1
#define DCMEZBUFF 1

/*
 this should NEVER be less than DICOMHEADERL + 4
*/
#define dcmezbuff_DICOMSIZEMAX (0x01 << 30)
#define dcmezbuff_DICOMHEADERL 128
#define dcmezbuff_DICOMFOURCC "DICM"

typedef struct
{
 unsigned int keLly;
 unsigned int p;
 byte1 *data;
} dcmbuff;

void dcmbuff_del(dcmbuff *todel)
{
 free(todel->data);
 free(todel);
}

/*
 get bytes from buff
*/
int dcmbuff_get(byte1 **current, dcmbuff *buff, unsigned int numchars)
{
 if(buff->keLly-buff->p < numchars)
  return dcmlog_log(l_write, NULL, "1:dcmbuff_get -- insufficient bytes to get", 0), 1;

 *current = &buff->data[buff->p];
 buff->p += numchars;

 return 0;
}

int dcmbuff_peek(byte1 **current, dcmbuff *buff, unsigned int numchars)
{
 if(buff->keLly-buff->p < numchars)
  return dcmlog_log(l_write, NULL, "1:dcmbuff_peek -- insufficient bytes to peek", 0), 1;

 *current = &buff->data[buff->p];

 return 0;
}

/*
int dcmezbuff_filetoobig(FILE *dicom)
{
 if(fseek(dicom, 0, SEEK_END)) return perror("1:dcmbuff_filetoobig -- could not determine file size"), 1;
 unsigned long int size = ftell(dicom);
 if(size == -1L) return perror("2:dcmbuff_filetoobig -- could not determine file size"), 2;
 if(size > dcmezbuff_DICOMSIZEMAX) return perror("3:dcmbuff_filetoobig -- file actually too big"), 3;

 return 0;
}
*/

/*
 loads dcmbuff from dicomfname
 opens and closes file pointer
*/
dcmbuff *dcmbuff_loaddicom(char *dicomfname)
{
 FILE *dicom = strcmp("-", dicomfname) ? fopen(dicomfname,"r") : stdin;
 if(!dicom)
  return dcmlog_log(l_write, NULL, "1:dcmbuff_loaddicom -- failed to open file", 0), NULL;

 byte1 *data;
 unsigned int nread;

 if(dicom == stdin)
 {
  data = (byte1*)malloc(dcmezbuff_DICOMSIZEMAX);
  if(!data)
   return dcmlog_log(l_write, NULL, "4:dcmbuff_loaddicom -- failed to allocate data", 0), NULL;

  nread = fread(data, 1, dcmezbuff_DICOMSIZEMAX, stdin);
  if(ferror(stdin))
   return dcmlog_log(l_write, NULL, "3:dcmbuff_loaddicom -- failed to read stdin", 0), NULL;

  if(!(data = realloc(data, nread)))
   return dcmlog_log(l_write, NULL, "6:dcmbuff_loaddicom -- failed to shrink data", 0), NULL;
 }
 else
 {
  if(fseek(dicom, 0, SEEK_END))
   return dcmlog_log(l_write, NULL, "2:dcmbuff_loaddicom -- file error", 0), NULL;
  long int size = ftell(dicom);
  if(size == -1L || size > dcmezbuff_DICOMSIZEMAX)
   return dcmlog_log(l_write, NULL, "3:dcmbuff_loaddicom -- file too big, or failed to determine size", 0), NULL;

  rewind(dicom);
  if(!(data = malloc(size)))
   return dcmlog_log(l_write, NULL, "4:dcmbuff_loaddicom -- failed to allocate data", 0), NULL;

  nread = fread(data, 1, size, dicom);
  if(ferror(dicom)) return perror("5:dcmbuff_loaddicom -- failed to read file"), NULL;

  fclose(dicom);
 }

 dcmbuff *buff;
 if(!(buff = malloc(sizeof(dcmbuff))))
  return dcmlog_log(l_write, NULL, "7:dcmbuff_loaddicom -- failed to allocate buff", 0), NULL;

 buff->data = data;
 buff->p = dcmezbuff_DICOMHEADERL;
 buff->keLly = nread;
 byte1 *tocheck;

 if
 (
  nread < dcmezbuff_DICOMHEADERL ||
  dcmbuff_get(&tocheck, buff, strlen(dcmezbuff_DICOMFOURCC)) ||
  strncmp(tocheck, dcmezbuff_DICOMFOURCC, strlen(dcmezbuff_DICOMFOURCC))
 ) return dcmlog_log(l_write, NULL, "8:dcmbuff_loaddicom -- file format error", 0), NULL;

 return buff;
}
