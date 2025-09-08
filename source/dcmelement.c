#ifndef _DCMTYPES
#include "dcmtypes.c"
#endif

#define dcmelement_ARRDEFAULTL 1024
#define dcmelement_ARRTOADD dcmelement_ARRDEFAULTL

/*
 tag Data Element Tag see part 5 section 7.1.1; converted to 4-byte integer
 vr Value Representation see part 5 section 7.1.1
 length Value Length see part 5 section 7.1.1
 data Value Field see part 5 section 7.1.1
 datastop should be equal to length unless length is 0xFFFFFFFF

 buffnum pos and datastop are depricated asof 20250908
*/
typedef struct
{
 unsigned long long buffnum; /* depricated */
 unsigned long long pos; /* depricated */
 byte4 tag;
 byte1 vr[2];
 byte4 length;
 byte1* rawmeta;
 byte1* data;
 unsigned long long datastop; /* depricated */
} dcmel;

/*
 free(&dcmel) is bad
*/
int dcmeldel(dcmel *element)
{
 free(element->data);
 free(element);
 return 0;
}

typedef struct
{
 unsigned int l;
 unsigned int p;
 dcmel **els;
} dcmelarr;

/*
 = 0: success
 = 1: parr is null
 = 2: failed to allocate memory
*/
int dcmelement_mkarr(dcmelarr **parr)
{
 if(parr == NULL) return 1;

 *parr = malloc(sizeof(dcmelarr));
 dcmelarr *arr = *parr;
 dcmel **els = (dcmel**)malloc(sizeof(dcmel*)*dcmelement_ARRDEFAULTL);
 if(arr == NULL || els == NULL) return 2;

 arr->els = els;
 arr->l = dcmelement_ARRDEFAULTL;
 arr->p = 0;

 return 0;
}

int dcmelement_addel(dcmelarr *arr, dcmel *el)
{
 if(arr == NULL || el == NULL) return 1;

 if(arr->p == arr->l)
 {
  void *newmem = realloc(arr->els, sizeof(void*)*(arr->l + dcmelement_ARRTOADD));
  if(newmem == NULL) return 2;

  arr->els = (dcmel**)newmem;
  arr->l += dcmelement_ARRTOADD;
 }
  
 arr->els[arr->p] = el;
 arr->p++;

 return 0;
}
