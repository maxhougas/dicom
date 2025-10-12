/*
 dcmtree.c

 functions for hanging nodes
*/

#ifndef DCMTYPES
#include "dcmtypes.c"
#endif
#ifndef DCMELEMENT
#include "dcmelement.c"
#endif
#ifndef DCMSPECIALTAG
#include "dcmspecialtag.c"
#endif

#define DCMTREE 1

#define dcmtree_UNDEFINEDLENGTH 0xFFFFFFFF
#define dcmtree_CHILDRENINITLENGTH 16 /* guessing a reasonable number of children / childable node */

/*
 DICOM standard part 5 section 7.5
*/
int dcmtree_recursivehang(dcmel **els)
{
 if(els == NULL) return perror("1:dcmtree_recursivehang"), 1;
 if(!dcmspecialtag_ischildable(*els)) return 0;

 (*els)->childarr = malloc(sizeof(dcmelarr));
 (*els)->childarr->els = malloc(sizeof(dcmel*)*dcmtree_CHILDRENINITLENGTH);
 if((*els)->childarr == NULL || (*els)->childarr->els == NULL) return perror("2:dcmtree_recursivehang -- failed to allocate childarr"), 2;

 dcmelarr *children = (*els)->childarr;
 children->p = 0;
 children->l = dcmtree_CHILDRENINITLENGTH;

 unsigned int i;

 if((*els)->length != dcmtree_UNDEFINEDLENGTH) /* the easy one */
 {
  byte4 bytesforward = 0;
  unsigned int istop;

  for(istop = 1; bytesforward < (*els)->length; istop++)
   bytesforward += els[istop]->effectivelength + els[istop]->metalength;
  for(i = 1; i < istop; i++)
  {
   if(els[i] == NULL) continue;

   if(children->p == children->l) /* expand */
   {
    children->els = realloc(children->els, sizeof(dcmel*) * (children->l + dcmtree_CHILDRENINITLENGTH));
    if(children->els == NULL) return perror("3:dcmtree_recursivehang -- failed to expand children->els"), 3;

    children->l += dcmtree_CHILDRENINITLENGTH;
   }
   if(dcmspecialtag_ischildable(els[i]))
    dcmtree_recursivehang(&els[i]);
   els[i]->parent = *els;
   children->els[children->p] = els[i];
   children->p++;
   els[i] = NULL;
  }
 }
 else /* look for ITEMDELIM or SEQUENCEDELIM i.e. tagstop */
 {
  byte4 tagstop = (*els)->tag == dcmspecialtag_ITEM ? dcmspecialtag_ITEMDELIM : dcmspecialtag_SEQUENCEDELIM;

  for(i = 1; els[i]->tag != tagstop; i++)
  {
   if(els[i] == NULL) continue;

   if(children->p == children->l) /* expand */
   {
    children->els = realloc(children->els, sizeof(dcmel*) * (children->l + dcmtree_CHILDRENINITLENGTH));
    if(children->els == NULL) return perror("3:dcmtree_recursivehang -- failed to expand children->els"), 3;

    children->l += dcmtree_CHILDRENINITLENGTH;
   }

   if(dcmspecialtag_ischildable(els[i]))
    dcmtree_recursivehang(&els[i]);
   els[i]->parent = *els;
   children->els[children->p] = els[i];
   children->p++;
   els[i] = NULL;
  }

  if(children->p == children->l) /* expand */
  {
   children->els = realloc(children->els, sizeof(dcmel*) * (children->l + dcmtree_CHILDRENINITLENGTH));
   if(children->els == NULL) return perror("3:dcmtree_recursivehang"), 3;

   children->l += dcmtree_CHILDRENINITLENGTH;
  }

  els[i]->parent = *els;
  children->els[children->p] = els[i];
  children->p++;
  els[i] = NULL;
 }

 if(children->p < children->l) /* shrink array */
 {
  children->els = realloc(children->els, sizeof(dcmel*) * children->p);
  if(children->els == NULL) return perror("4:dcmtree_recursivehang -- failed to shrink children->els"), 4;

  children->l = children->p;
 }

 return 0;
}

/*
 Trim trailing nulls from dcmarr after recursivehang
*/
int dcmtree_trim(dcmelarr *arr)
{
 if(arr == NULL || arr->els == NULL) return perror("1:dcmtree_trim -- arr is null"), 1;

 unsigned int i;
 for(i = arr->p - 1; arr->els[i] == NULL && i > 0; i--);
 if(i == 0 && arr->els[0] == NULL) return perror("2:dcmtree_trim -- arr->els is empty"), 2;
 arr->p = i + 1;

 arr->els = realloc(arr->els, sizeof(dcmel*) * arr->p);
 if(arr->els == NULL) return perror("3:dcmtree_trim -- failed to shrink arr->els"), 3;

 return 0;
}
