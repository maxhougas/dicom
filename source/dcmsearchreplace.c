/*
 dcmsearchreplace.c

 tools for searching for dcm elements and replacing their contents
*/

#ifndef DCMELEMENT
#include "dcmelement.c"
#endif
#ifndef DCMEZBUFF
#include "dcmezbuff.c"
#endif

#define DCMSEARCHREPLACE 1

/*
 get a dcmelarr of matches
*/
int dcmsearchreplace_search(dcmelarr **found, dcmelarr *unrecursed, byte4 tag)
{
 if(unrecursed == NULL) return perror("1:dcmsearchreplace_search -- unrecursed is null"), 1;

 dcmelement_mkarr(found);

 unsigned int i;
 for(i = 0; i < unrecursed->p; i++)
 {
  if(unrecursed->els[i]->tag == tag)
  {
   if((*found)->p == (*found)->l)
   {
    (*found)->els = realloc((*found)->els, sizeof(dcmel) * ((*found)->l + dcmelement_ARRTOADD));
    if((*found->els == NULL) return perror("2:dcmsearchreplace_search -- failed to expand found->els"), 2;
    (*found)->l += dcmelement_ARRTOADD;
   }

   (*found)->els[p] = unrecursed->els[i];
   (*found)->p++;
  }

  if((*found)->p < (*found)->l)
  {
   (*found)->els = realloc((*found)->els, sizeof(dcmel) * (*found)->p);
   if((*found)->els = NULL) return perror("3:dcmsearchreplace_search -- failed to shrink found->els"), 3;
   (*found)->l = (*found)->p;
  }

  return 0;
 }
}
