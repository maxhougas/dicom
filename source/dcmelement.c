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

#define dcmelement_ARRDEFAULTL 0x400
#define dcmelement_ARRTOADD dcmelement_ARRDEFAULTL

/*
 tag Data Element Tag see part 5 section 7.1.1; converted to 4-byte integer
 vr Value Representation see part 5 section 7.1.1
 length Value Length see part 5 section 7.1.1
 data Value Field see part 5 section 7.1.1
 datastop should be equal to length unless length is 0xFFFFFFFF

 buffnum pos and datastop are depricated asof 20250908
*/

typedef struct dcmelarr dcmelarr;
typedef struct dcmel dcmel;

struct dcmel
{
 byte4 tag;
 byte1 vr[2];
 byte1 metalength;
 byte4 length;
 byte4 effectivelength;
 byte1* rawmeta;
 byte1* data;

 dcmel *parent;
 dcmelarr *childarr;

/*
 struct dcmel** children;
 byte4 nchildren;
*/
};

struct dcmelarr
{
 unsigned int l;
 unsigned int p;
 dcmel **els;
};

int dcmelement_delel(dcmel *el);
int dcmelement_delarr(dcmelarr *arr);

/*
 free(&dcmel) is bad
*/
int dcmelement_delel(dcmel *el)
{
 if(el == NULL) return 0;

 free(el->rawmeta);

 if(el->childarr != NULL)
  dcmelement_delarr(el->childarr);
 else
  free(el->data);
 
 free(el);

 return 0;
}

/*
 initialize dcmelarr
*/
int dcmelement_mkarr(dcmelarr **parr)
{
 if(parr == NULL) return perror("1:dcmelement_mkarr -- parr is null"), 1;

 *parr = malloc(sizeof(dcmelarr));
 if(*parr == NULL) return perror("2:dcmelement_mkarr -- failed to allocate *parr"), 2;

 (*parr)->els = (dcmel**)malloc(sizeof(dcmel*)*dcmelement_ARRDEFAULTL);
 if((*parr)->els == NULL) return perror("3:dcmelement_mkarr -- failed to allocate *parr->els"), 3;

 (*parr)->l = dcmelement_ARRDEFAULTL;
 (*parr)->p = 0;

 return 0;
}

/*
 free(dcmelarr) is bad
*/
int dcmelement_delarr(dcmelarr *arr)
{
 if(arr == NULL) return 0;

 unsigned int i;

 for(i = 0; i < arr->p; i++)
 {
  if(arr->els[i] == NULL) continue;
  dcmelement_delel(arr->els[i]);
 }

 free(arr->els);
 free(arr);

 return 0;
}

int dcmelement_addel(dcmelarr *arr, dcmel *el)
{
 if(arr == NULL || arr->els == NULL || el == NULL) return perror("1:dcmelement_addel"), 1;

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
