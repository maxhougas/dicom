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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 0x01 << 10 = ki
 0x01 << 20 = Mi
 0x01 << 30 = Gi
 iff int is 4 B and 1 B = 8 b, signed int can address 2 GiB
*/
#define dcmsmartbuff_BUFFERSIZE (0x01 << 10)
#define dcmsmartbuff_MAXBUFFERSIZE (0x01 << 30)

typedef struct
{
 unsigned int num;
 char* data;
 int l;
 unsigned long long p;
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
 =  1: rawbuffexpand failed
 =  2: unspecified file read error
*/
int dcmsmartbuff_expand(dcmbuff *buff, FILE* dicom)
{
 char *newdata;
 int *l = &buff->l;
 int *p = &buff->p;
 char *olddata = &buff->data[*p];

 if(dcmsmartbuff_rawbuffexpand(&newdata, olddata, *l-*p)) return 1;

 int nread = fread(&newdata, 1, dcmsmartbuff_BUFFERSIZE, dicom)
 if(ferror(dicom)) return 2;

 *l = *l - *p + nread;
 *p = 0;
 free(buff->data);
 buff->data = newdata;

 if(feof(dicom)) return -1;

 return 0;
}

int dcmbuff_get(char **current, dcmbuff *buff, int numchars)
{
}

/*
 = -1: eof encountered
 =  0: success
 =  1: null parameter(s)
 =  2: failed to allocate memory
 =  3: unspecified file read error
*/
int dcmbuff_loaddicom(dcmbuff **buff, FILE *dicom)
{
 if(buff == NULL || dicom == NULL) return 1;
 
 char *data = malloc(dcmsmartbuff_BUFFERSIZE);
 *buff = malloc(sizeof(dcmbuff));
 if(data == NULL || *buff == NULL) return 2;

 buff->data = data;
 buff->p = 0;

 int nread = fread(&data, 1, dcmsmartbuff_BUFFSIZE);
 if(ferror(dicom)) return 3;

 buff->l = nread;

 if(feof(dicom)) return -1;

 return 0;
}
