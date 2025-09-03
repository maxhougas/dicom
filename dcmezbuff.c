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

#include <stdio.h>
#include <stdlib.h>

#define dcmezbuff_DICOMSIZEMAX (0x01 << 30)

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

int dcmbuff_get(char **current, dcmbuff *buff, int numchars)
{
 if(buff == NULL || buff->data == NULL) return 1;
 if(p + numchars >= buff->l || numlchars < 0) return 2;

 *current = buff->data[buff->p];
 p += numchars;

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
 buff->p = 0;

 int nread = fread(data, 1, size, dicom);
 if(ferror(dicom)) return 5;

 buff->l = nread;

 if(feof(dicom)) return -1;

 return 0;
}
