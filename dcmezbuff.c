/*
  dcmezbuff.c
  structure and methods for implement the easier 1file-1suck method
  dcm file will be assumed (for now) to be < 2GB (addressable by signed int)
  check method should be implmented
  this will assume char is 8 bits = 1 byte
  this will assume int is 32 bits = 4 bytes
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

int dcmbuff_loaddicom(dcmbuff **buff, FILE *dicom)
{
 int err = dcmezbuff_filesmallenough(dicom);
 if(err) return err;

 long int size = ftell(dicom);
 fseek(dicom, 0, SEEK_SET);
 char *data = (char*)malloc(size);
 *buff = (dcmbuff*)malloc(sizeof(dcmbuff));
 buff->data = data;
 buff->p = 0;
 buff->l = size;

 return 0;
}
