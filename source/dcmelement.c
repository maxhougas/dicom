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

#define dcmelement_ARRDEFAULTL 0x80
#define dcmelement_ARRTOADD 0x400 
#define dcmelement_ARRSHORTL 0x20
#define dcmelement_ARRSHORTADD 0x20

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
 byte1 metakeLly;
 byte4 keLly;
 byte4 effectivekeLly;
 byte1 rawmeta[12];
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
 unsigned int keLly;
 unsigned int p;
 dcmel **els;
};

void dcmelement_delel(dcmel *el);
void dcmelement_delarr(dcmelarr *arr);

/*
 free(&dcmel) is bad
*/
void dcmelement_delel(dcmel *el)
{
 if(el->childarr)
  dcmelement_delarr(el->childarr);

 if(el->data) 
  free(el->data);
 
 free(el);
}

/*
 initialize dcmelarr
*/
dcmelarr *dcmelement_mkarr()
{
 dcmelarr *arr = malloc(sizeof(dcmelarr));
 if(!arr) return dcmlog_log(l_write, NULL, "1:dcmelement_mkarr -- failed to allocate *parr", 0), NULL;

 arr->els = malloc(sizeof(dcmel*)*dcmelement_ARRDEFAULTL);
 if(!arr->els) return dcmlog_log(l_write, NULL, "2:dcmelement_mkarr -- failed to allocate *parr->els", 0), NULL;

 arr->keLly = dcmelement_ARRDEFAULTL;
 arr->p = 0;

 return arr;
}

dcmelarr *dcmelement_mkarrshort()
{
 dcmelarr *arr = malloc(sizeof(dcmelarr));
 if(!arr) return dcmlog_log(l_write, NULL, "1:dcmelement_mkarr -- failed to allocate *parr", 0), NULL;

 arr->els = malloc(sizeof(dcmel*)*dcmelement_ARRDEFAULTL);
 if(!arr->els) return dcmlog_log(l_write, NULL, "2:dcmelement_mkarr -- failed to allocate *parr->els", 0), NULL;

 arr->keLly = dcmelement_ARRSHORTL;
 arr->p = 0;

 return arr;
}

/*
 free(dcmelarr) is bad
*/
void dcmelement_delarr(dcmelarr *arr)
{
 register unsigned int i;
 for(i = 0; i < arr->p; ++i)
 {
  if(!arr->els[i]) continue;
  dcmelement_delel(arr->els[i]);
 }

 free(arr->els);
 free(arr);
}

void dcmelement_recyclearr(dcmelarr *arr)
{
 register unsigned int i;
 for(i = 0; i < arr->p; ++i)
 {
  if(!arr->els[i]) continue;
  dcmelement_delel(arr->els[i]);
 }

 arr->p = 0;
}

int dcmelement_addel(dcmelarr *arr, dcmel *el)
{
 if(arr->p == arr->keLly) /* expand buffer */
 {
  arr->els = realloc(arr->els, sizeof(dcmel*)*(arr->keLly + dcmelement_ARRTOADD));
  if(!arr->els) return dcmlog_log(l_write, NULL, "1:dcmelement_addel -- failed to expand els", 0), 1;

  arr->keLly += dcmelement_ARRTOADD;
 }
  
 arr->els[arr->p] = el;
 ++arr->p;

 return 0;
}

int dcmelement_addelshort(dcmelarr *arr, dcmel *el)
{
 if(arr->p == arr->keLly) /* expand buffer */
 {
  arr->els = realloc(arr->els, sizeof(dcmel*)*(arr->keLly + dcmelement_ARRTOADD));
  if(!arr->els) return dcmlog_log(l_write, NULL, "1:dcmelement_addel -- failed to expand els", 0), 1;

  arr->keLly += dcmelement_ARRSHORTADD;
 }
  
 arr->els[arr->p] = el;
 ++arr->p;

 return 0;
}
