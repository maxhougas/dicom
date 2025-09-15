/*
 dcmtree.c

 functions for hanging nodes
*/

#ifndef _DCMTYPES
#include "dcmtypes.c"
#endif
#ifndef _DCMELEMENT
#include "dcmelement.c"
#endif
#ifndef _DCMSPECIALTAG
#include "dcmspecialtag.c"
#endif

#define _DCMTREE 1

#define dcmtree_UNDEFINEDLENGTH 0xFFFFFFFF
#define dcmtree_CHILDRENINITLENGTH 16 /* guessing a reasonable number of children / childable node */

/*
 DICOM standard part 5 section 7.5
*/
int dcmtree_recursivehang(dcmel **els)
{
 if(els == NULL) return perror("1:dcmtree_recursivehang"), 1;

 if(!dcmspecialtag_ischildable(*els)) return 0;

 (*els)->children = (dcmel**)malloc(sizeof(dcmel*)*dcmtree_CHILDRENINITLENGTH);
 if((*els)->children == NULL) return perror("2:dcmtree_recursivehang"), 2;

 unsigned int maxchildren = dcmtree_CHILDRENINITLENGTH;
 unsigned int i;

 if((*els)->length != dcmtree_UNDEFINEDLENGTH) /* the easy one */
 {
  byte4 bytesforward = 0;
  unsigned int istop = 0;

  for(istop = 1; bytesforward < (*els)->length; istop++)
   bytesforward += dcmspecialtag_ischildable(els[istop]) ? els[istop]->metalength : els[istop]->length + els[istop]->metalength;
  for(i = 1; i < istop; i++)
  {
   if(els[i] == NULL) continue;
   if(i - 1 == maxchildren) /* expand */
   {
    (*els)->children = (dcmel**)realloc((*els)->children, maxchildren + sizeof(dcmel*)*dcmtree_CHILDRENINITLENGTH);
    if((*els)->children == NULL) return perror("3:dcmtree_recursivehang"), 3;

    maxchildren += dcmtree_CHILDRENINITLENGTH;
   }
   if(dcmspecialtag_ischildable(els[i]))
    dcmtree_recursivehang(&els[i]);
   (*els)->children[(*els)->nchildren] = els[i];
   (*els)->nchildren++;
   els[i] = NULL;
  }
 }
 else /* look for ITEMDELIM or SEQUENCEDELIM i.e. tagstop */
 {
  byte4 tagstop = (*els)->tag == dcmspecialtag_ITEM ? dcmspecialtag_ITEMDELIM : dcmspecialtag_SEQUENCEDELIM;

  for(i = 1; els[i]->tag != tagstop; i++)
  {
   if(els[i] == NULL) continue;
   if(i - 1 == maxchildren) /* expand */
   {
    (*els)->children = (dcmel**)realloc((*els)->children, maxchildren + sizeof(dcmel*)*dcmtree_CHILDRENINITLENGTH);
    if((*els)->children == NULL) return perror("3:dcmtree_recursivehang"), 3;

    maxchildren += dcmtree_CHILDRENINITLENGTH;
   }

   if(dcmspecialtag_ischildable(els[i]))
    dcmtree_recursivehang(&els[i]);
   (*els)->children[(*els)->nchildren] = els[i];
   (*els)->nchildren++;
   els[i] = NULL;
  }

  if(i - 1 == maxchildren) /* expand */
  {
   (*els)->children = (dcmel**)realloc((*els)->children, maxchildren + sizeof(dcmel*)*dcmtree_CHILDRENINITLENGTH);
   if((*els)->children == NULL) return perror("3:dcmtree_recursivehang"), 3;

   maxchildren += dcmtree_CHILDRENINITLENGTH;
  }

  (*els)->children[(*els)->nchildren] = els[i];
  (*els)->nchildren ++;
  els[i] = NULL;
 }

 if((*els)->nchildren < maxchildren)
 {
  (*els)->children = (dcmel**)realloc((*els)->children, sizeof(dcmel*)*(*els)->nchildren);
  if((*els)->children == NULL) return perror("4:dcmtree_recursivehang"), 4;
 }
}
