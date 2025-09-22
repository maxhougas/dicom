/*
 dcmelement.c

 structures for containing parsed dicom data
 dcmel is self explanitory
 dcmelarr is an array of dcmel*
 dcmelement_mkarr initializes a dcmarr
 dcmelement_addel adds an element to the array, expanding if necessary
 renaming may be in order here
*/

#ifndef DCMTYPES
#include "dcmtypes.c"
#endif

#define DCMELEMENT 1

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
typedef struct dcmel
{
 byte4 tag;
 byte1 vr[2];
 byte1 metalength;
 byte4 length;
 byte4 effectivelength;
 byte1* rawmeta;
 byte1* data;
 struct dcmel** children;
 byte4 nchildren;
} dcmel;

/*
 free(&dcmel) is bad
*/
int dcmeldel(dcmel *element)
{
 free(element->rawmeta);
 if(element->nchildren)
 {
  unsigned int i;
  for(i = 0; i < element->nchildren; i++)
   dcmeldel(element->children[i]);
  free(element->children);
 }
 else
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
 if(parr == NULL) return perror("1:dcmelement_mkarr"), 1;

 *parr = malloc(sizeof(dcmelarr));
 if(*parr == NULL) return perror("2:dcmelement_mkarr"), 2;

 (*parr)->els = (dcmel**)malloc(sizeof(dcmel*)*dcmelement_ARRDEFAULTL);
 if((*parr)->els == NULL) return perror("3:dcmelement_mkarr"), 3;

 (*parr)->l = dcmelement_ARRDEFAULTL;
 (*parr)->p = 0;

 return 0;
}

/*
 free(dcmelarr) is bad
*/
int dcmelement_delarr(dcmelarr *arr)
{
 if(arr == NULL || arr->els == NULL) return perror("1:dcmelement_delarr"), 1;

 unsigned int i;

 for(i = 0; i < arr->p; i++)
 {
  if(arr->els[i] == NULL) continue;
  dcmeldel(arr->els[i]);
 }

 free(arr->els);
 free(arr);

 return 0;
}

int dcmelement_addel(dcmelarr *arr, dcmel *el)
{
 if(arr == NULL || el == NULL) return perror("1:dcmelement_addel"), 1;

 if(arr->p == arr->l) /* expand buffer */
 {
  arr->els = realloc(arr->els, sizeof(dcmel*)*(arr->l + dcmelement_ARRTOADD));
  if(arr->els == NULL) return perror("2:dcmelement_addel"), 2;

  arr->l += dcmelement_ARRTOADD;
 }
  
 arr->els[arr->p] = el;
 arr->p++;

 return 0;
}
