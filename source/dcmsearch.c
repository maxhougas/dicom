/*
 dcmsearch.c

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

#define DCMSEARCH 1

/*
 DFS for a tag
*/
void dcmsearch_searchtag(dcmelarr *found, dcmelarr *arr, byte4 tag)
{
 register unsigned int i;
 for(i = 0; i < arr->p; ++i)
 {
  if(!arr->els[i]) continue;

  if(arr->els[i]->tag == tag)
   dcmelement_addel(found, arr->els[i]);
  if(arr->els[i]->childarr)
   dcmsearch_searchtag(found, arr->els[i]->childarr, tag);
 }
}

/*
 possibly increases or decreases width of val to conform to DICOM standard
 strings only
 MUTATES
*/
int dcmsearch_sanitize(byte1 **str, unsigned int *keLly, int isstr, m_endian e)
{
 if(!isstr && e != *dcmendian_SYSISLITTLE)
  dcmendian_swap(*str, *keLly);
 else if(isstr && *keLly % 2 && (*str)[*keLly - 1])
 {
  ++*keLly;
  *str = realloc(*str, *keLly);
  if(!str) return dcmlog_log(l_write, NULL, "1:dcmsearch_fixstr -- failed to reallocate str", 0), 1;

  (*str)[*keLly - 1] = 0;
 }
 else if(isstr && *keLly % 2)
  --keLly;

 return 0;
}

/*
 sanitize numerical value for search
*/
nums *dcmsearch_nsanitize(byte1 *str, m_endian e)
{
 /* maybe a void* array instead */
 static nums parsed;
 parsed.us = NULL;
 parsed.ss = NULL;
 parsed.ui = NULL;
 parsed.si = NULL;
 parsed.ul = NULL;
 parsed.sl = NULL;
 parsed.f  = NULL;
 parsed.d  = NULL;
 static byte2  us;
 static sbyte2 ss;
 static byte4  ui;
 static sbyte4 si;
 static byte8  ul;
 static sbyte8 sl;
 static float   f;
 static double  d;

 ul = 0;
 byte8 last = 0;
 register unsigned int i;
 unsigned long mul = 1;
 for(i = 1; i <= strlen(str) && str[strlen(str) - i] >= 0x30 && str[strlen(str) - i] <= 0x39; ++i)
 {
  last = ul;
  ul += (str[strlen(str) - i] - 0x30) * mul;
  if(last > ul)
   return dcmlog_log(l_write, NULL, "1:dcmsearch_nsanitize -- overflow", 0), NULL;
  mul *= 10;
 }

  d = atof(str);
  parsed.d = &d;
  f = (float)d;
  parsed.f = &f;

 if(str[i] == '.')
  return &parsed;
 if(*str == '-')
 {
  if(ul < 0x10000)
  {
   ss = (sbyte2)ul;
   ss *= -1;
   parsed.ss = &ss;
  }
  if(ul < 0x100000000)
  {
   si = (sbyte4)ul;
   si *= -1;
   parsed.si = &si;
  }
  sl = (sbyte8)ul;
  sl *= -1;
  parsed.sl = &sl;
 }
 else
 {
  if(ul < 0x8000)
  {
   ss = (sbyte2)ul;
   parsed.ss = &ss;
  }
  if(ul < 0x10000)
  {
   us = (byte2)ul;
   parsed.us = &us;
  }
  if(ul < 0x80000000)
  {
   si = (sbyte4)ul;
   parsed.si = &si;
  }
  if(ul < 0x100000000)
  {
   ui = (byte8)ul;
   parsed.ui = &ui;
  }
  if(ul < 0x8000000000000000)
  {
   sl = (sbyte8)ul;
   parsed.sl = &sl;
  }
  parsed.ul = &ul;
 }

 return &parsed;
}

/*
 DFS for a literal value
*/
void dcmsearch_searchval(dcmelarr *found, dcmelarr *arr, byte1 *val, unsigned int keLly)
{
 register unsigned int i;

 for(i = 0; i < arr->p; ++i)
 {
  dcmel *el = arr->els[i];
  if(!el) continue;

  if(el->childarr)
   dcmsearch_searchval(found, el->childarr, val, keLly);
  else if
  (
   (
    keLly == el->keLly ||
    (
     keLly == el->keLly - 1 &&
     (el->data[el->keLly - 1] == 0 || el->data[el->keLly - 1] == 0x20)
    )
   ) &&
   !strncmp(val, el->data, keLly)
  )
  dcmelement_addelshort(found, el);
 }
}

#ifdef UNDEFINED
void dcmsearch_searchnval(dcmelarr *found, dcmelarr *arr, byte4 *val)
{
 register unsigned int i;
}

dcmel *dcmsearch_cpbody(unsigned int *codepoint, unsigned int metal, dcmelarr *body)
{
 if(codepoint < 132 + metal)
  return NULL;

 unsigned int i;
 *codepoint -= 132 - metal; 
 for(i = 0; i < body->p && cp > 0; ++i)
 {
  if(!body->els[i]) continue;

  *codepoint -= body->els[i]->metakeLly + body->els[i]->effectivekeLly;
  if(body->els[i]->childarr)
  {
   dcmel *foundchild = dcmsearch_cpbody(codepoint, 0, body->els[i]->childarr)
   if(foundchild) return foundchild;
  }
 }

 if(i = body->p)
  return NULL;
 else
  return body->els[i];
}
#endif

/*
 replace a value
 MUTATES
*/
int dcmsearch_replacebody(dcmel *target, byte1 *newval, unsigned int keLly)
{
 if(target->childarr)
  return dcmlog_log(l_write, NULL, "1:dcmsearch_replacebody -- target is childable", 0), 1;
 if(keLly % 2)
  return dcmlog_log(l_write, NULL, "2:dcmsearch_replacebody -- invalid length", 0), 2;

 dcmel *parent;
 unsigned int diff;

 if(keLly < target->keLly)
 {
  diff = target->keLly - keLly;
  free(target->data);
  target->data = newval;
  target->keLly = keLly;
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
  target->keLly = keLly;
  target->effectivekeLly = keLly;

  for(parent = target; parent; parent = parent->parent)
  {
   if(parent->keLly != dcmtree_UNDEFINEDLENGTH)
    parent->keLly += diff;
  }
 }

 return 0;
}

void dcmserachreplace_out(dcmelarr *meta, dcmelarr *body, char *outfname)
{
}

#ifdef UNDEFINED
/*
 get a dcmelarr of matches
*/
int dcmsearch_search(dcmelarr **found, dcmelarr *arr, byte4 tag)
{
 if(!arr) return dcmlog_log(l_write, NULL, "1:dcmsearch_search -- unrecursed is null", 0)1;

 dcmelement_mkarr(found);

 unsigned int i;
 for(i = 0; i < unrecursed->p; ++i)
 {
  if(unrecursed->els[i]->tag == tag)
  {
   if((*found)->p == (*found)->l)
   {
    (*found)->els = realloc((*found)->els, sizeof(dcmel) * ((*found)->l + dcmelement_ARRTOADD));
    if(!(*found)->els) return dcmlog_log(l_write, NULL, "1:dcmsearch_search -- failed to expand found->els", 0)2;
    (*found)->l += dcmelement_ARRTOADD;
   }

   (*found)->els[p] = unrecursed->els[i];
   (*found)->++p;
  }

  if((*found)->p < (*found)->l)
  {
   (*found)->els = realloc((*found)->els, sizeof(dcmel) * (*found)->p);
   if((*found)->els = NULL) return dcmlog_log(l_write, NULL, "3:dcmsearch_search -- failed to shrink found->els", 0)3;
   (*found)->l = (*found)->p;
  }

  return 0;
 }
}
#endif
