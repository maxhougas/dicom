/*
 dcmsmartbuff.c
 reads dicom file part-by-part
 asof now this assumes 1 B = 8 b characters
 should implement
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

#ifndef _STRING_H
#include <string.h>
#endif

/*
 this should not be less than DICOMHEADERL + 4
*/
#define dcmsmartbuff_BUFFERSIZE (0x01 << 10)
#define dcmsmartbuff_MAXBUFFERSIZE (0x01 << 30)
#define dcmsmartbuff_DICOMHEADERL 128
#define dcmsmartbuff_DICOMFOURCC "DICM"

typedef struct
{
 unsigned int num;
 char *data;
 int l;
 int p;
 FILE *dicom;
} dcmbuff;

void dcmbuff_del(dcmbuff *buff)
{
 free(buff->data);
 free(buff);
}

/*
 Does not free(olddata)
 = 0: success
 = 1: too many data
 = 2: failed to allocate new buffer 
*/
int dcmsmartbuff_rawbuffexpand(char **newdata, char *olddata, int sizeolddata)
{
 if(dcmsmartbuff_BUFFERSIZE + sizeolddata > dcmsmartbuff_MAXBUFFERSIZE) return 1;

 *newdata = (char*)malloc(dcmsmartbuff_BUFFERSIZE + sizeolddata);
 if(*newdata == NULL) return 2;

 memcpy(newdata, olddata, sizeolddata);
 
 return 0;
} 

/*
 = -1: encountered eof
 =  0: success
 =  1: null parameter(s)
 =  2: rawbuffexpand failed
 =  3: unspecified file read error
*/
int dcmsmartbuff_expand(dcmbuff *buff)
{
 if(buff == NULL || buff->data == NULL || buff->dicom || NULL) return 1;

 char *newdata;
 int *l = &buff->l;
 int *p = &buff->p;
 char *olddata = &buff->data[*p];
 if(dcmsmartbuff_rawbuffexpand(&newdata, olddata, *l-*p)) return 2;

 FILE *dicom = buff->dicom;
 int nread = fread(&newdata, 1, dcmsmartbuff_BUFFERSIZE, dicom)
 if(ferror(dicom)) return 3;

 *l = *l - *p + nread;
 *p = 0;
 free(buff->data);
 buff->data = newdata;

 if(feof(dicom)) return -1;

 return 0;
}

int dcmbuff_get(char **current, dcmbuff *buff, int numchars)
{
 if(*current == NULL || buff == NULL || buff->data == NULL) return 1;

 int *l = &buff->l;
 int *p = &buff->p;
 int expandfail;
 for(;*l-*p < numchars; expandfail = dcmsmartbuff_expand(buff))
  if(expandfail && *l-*p < numchars) return 2;

 current = &buff->data[*p];
 *p += numchars;

 return 0;
}

/*
 = -1: eof encountered
 =  0: success
 =  1: null parameter(s)
 =  2: failed to allocate memory
 =  3: unspecified file read error
 =  4: fourcc check failed
*/
int dcmbuff_loaddicom(dcmbuff **buff, FILE *dicom)
{
 if(buff == NULL || dicom == NULL) return 1;
 
 char *data = malloc(dcmsmartbuff_BUFFERSIZE);
 *buff = malloc(sizeof(dcmbuff));
 if(data == NULL || *buff == NULL) return 2;

 buff->data = data;
 buff->dicom = dicom;

 int nread = fread(&data, 1, dcmsmartbuff_BUFFSIZE);
 if(ferror(dicom)) return 3;

 buff->p = dcmezbuff_DICOMHEADERL;
 char* tocheck;
 if
 (
  nread < dcmsmartbuff+DICOMHEADERL ||
  dcmbuff_get(&tocheck, *buff, strlen(dcmsmartbuff_DICOMFOURCC)) ||
  strncmp(tocheck, dcmsmartbuff_DICOMFOURCC, strlen(dcmsmartbuff_DICOMFOURCC))
 ) return 4;

 buff->l = nread;

 if(feof(dicom)) return -1;

 return 0;
}
