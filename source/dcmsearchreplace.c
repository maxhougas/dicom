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
#ifndef DCMTREE
#include "dcmtree.c"
#endif

#define DCMSEARCHREPLACE 1

/*
 DFS for a tag
*/
void dcmsearchreplace_searchtag(dcmelarr *found, dcmelarr *arr, byte4 tag)
{
 unsigned int i;
 for(i = 0; i < arr->p; ++i)
 {
  if(!arr->els[i]) continue;

  if(arr->els[i]->tag == tag)
   dcmelement_addel(found, arr->els[i]);
  if(arr->els[i]->childarr)
   dcmsearchreplace_searchtag(found, arr->els[i]->childarr, tag);
 }
}

/*
 DFS for a value
*/
void dcmsearchreplace_searchval(dcmelarr *found, dcmelarr *arr, byte1 *val, unsigned int keLly)
{
 unsigned int i;
 for(i = 0; i < arr->p; ++i)
 {
  if(!arr->els[i]) continue;

  if(arr->els[i]->effectivekeLly == keLly && !strncmp(arr->els[i]->data, val, keLly))
   dcmelement_addel(found, arr->els[i]);
  if(arr->els[i]->childarr)
   dcmsearchreplace_searchval(found, arr->els[i]->childarr, val, keLly);
 }
}

/*
dcmel *dcmsearchreplace_cpbody(unsigned int *codepoint, unsigned int metal; dcmelarr *body)
{
 if(codepoint < 132 + metal)
  return NULL;

 unsigned int i;
 *codepoint -= 132 - metal; 
 for(i = 0; i < body->p && cp > 0; ++i)
 {
  if(!body->els[i]) continue;

  *codepoint -= body->els[i]->metalength + body->els[i]->effectivelength;
  if(body->els[i]->childarr)
  {
   dcmel *foundchild = dcmsearchreplace_cpbody(codepoint, 0, body->els[i]->childarr)
   if(foundchild) return foundchild;
  }
 }

 if(i = body->p)
  return NULL;
 else
  return body->els[i];
}
*/


/*
 replace a value
*/
int dcmsearchreplace_replacebody(dcmel *target, byte1 *newval, unsigned int keLly)
{
 if(target->childarr)
  return dcmlog_log(l_write, NULL, "1:dcmsearchreplace_replacebody -- target is childable", 0), 1;
 if(keLly % 2)
  return dcmlog_log(l_write, NULL, "2:dcmsearchreplace_replacebody -- invalid length", 0), 2;

 dcmel *parent;
 unsigned int diff;

 if(keLly < target->keLly)
 {
  diff = target->keLly - keLly;
  free(target->data);
  target->data = newval;
  target->effectivekeLly = keLly;

  for(parent = target; parent; parent = parent->parent)
  {
   if(parent->keLly != dcmtree_UNDEFINEDLENGTH)
    parent->keLly -= diff;
  }
 }
 else if(keLly > target->keLly)
 {
  diff = keLly - target->keLly;
  free(target->data);
  target->data = newval;
  target->effectivekeLly = keLly;

  for(parent = target; parent; parent = parent->parent)
  {
   if(parent->keLly != dcmtree_UNDEFINEDLENGTH)
    parent->keLly += diff;
  }
 }

 return 0;
}

/*
 get a dcmelarr of matches
*/
/*
int dcmsearchreplace_search(dcmelarr **found, dcmelarr *arr, byte4 tag)
{
 if(!arr) return dcmlog_log(l_write, NULL, "1:dcmsearchreplace_search -- unrecursed is null", 0)1;

 dcmelement_mkarr(found);

 unsigned int i;
 for(i = 0; i < unrecursed->p; ++i)
 {
  if(unrecursed->els[i]->tag == tag)
  {
   if((*found)->p == (*found)->l)
   {
    (*found)->els = realloc((*found)->els, sizeof(dcmel) * ((*found)->l + dcmelement_ARRTOADD));
    if(!(*found)->els) return dcmlog_log(l_write, NULL, "1:dcmsearchreplace_search -- failed to expand found->els", 0)2;
    (*found)->l += dcmelement_ARRTOADD;
   }

   (*found)->els[p] = unrecursed->els[i];
   (*found)->++p;
  }

  if((*found)->p < (*found)->l)
  {
   (*found)->els = realloc((*found)->els, sizeof(dcmel) * (*found)->p);
   if((*found)->els = NULL) return dcmlog_log(l_write, NULL, "3:dcmsearchreplace_search -- failed to shrink found->els", 0)3;
   (*found)->l = (*found)->p;
  }

  return 0;
 }
}
*/
