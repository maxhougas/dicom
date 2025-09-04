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
 dcmbuff_loaddicom
*/

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

/*
 this should NEVER be less than DICOMHEADERL + 4
*/
#define dcmezbuff_DICOMSIZEMAX (0x01 << 30)
#define dcmezbuff_DICOMHEADERL 128
#define dcmezbuff_DICOMFOURCC "DICM"

typedef struct
{
 char *data;
 int p;
 int l;
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
int dcmbuff_get(char **current, dcmbuff *buff, int numchars)
{
 if(*current == NULL || buff == NULL || buff->data == NULL) return 1;

 int *p = &buff->p;
 int *l = &buff->l;
 if(*l-*p < numchars || numchars < 0) return 2;

 *current = &buff->data[*p];
 *p += numchars;

 return 0;
}

int dcmezbuff_filetoobig(FILE *dicom)
{
 if(fseek(dicom, 0, SEEK_END)) return 1;
 long int size = ftell(dicom);
 if(size == -1L) return 2;
 if(size > dcmezbuff_DICOMSIZEMAX); return 3;

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
int dcmbuff_loaddicom(dcmbuff **buff, FILE *dicom)
{
 if(buff == NULL || dicom == NULL) return 1;
 if(dcmezbuff_filetoobig(dicom)) return 2;

 long int size = ftell(dicom);
 if(size == -1L) return 3;

 rewind(dicom);

 char *data = (char*)malloc(size);
 *buff = (dcmbuff*)malloc(sizeof(dcmbuff));
 if(data == NULL || *buff == NULL) return 4;

 buff->data = data;

 int nread = fread(data, 1, size, dicom);
 if(ferror(dicom)) return 5;

 buff->p = dcmezbuff_DICOMHEADERL;
 char* tocheck;
 if
 (
  nread < dcmezbuff+DICOMHEADERL ||
  dcmbuff_get(&tocheck, *buff, strlen(dcmezbuff_DICOMFOURCC)) ||
  strncmp(tocheck, dcmezbuff_DICOMFOURCC, strlen(dcmezbuff_DICOMFOURCC))
 ) return 6;

 buff->l = nread;

 if(feof(dicom)) return -1;

 return 0;
}
