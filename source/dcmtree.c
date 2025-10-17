/*
 dcmtree.c

 tools for building trees of elements
*/

#ifndef _STDIO_H
#include <stdio.h>
#endif
#ifndef _STDLIB_H
#include <stdlib.h>
#endif
#ifndef _STRING_H
#include <string.h>
#endif

#ifndef DCMTYPES
#include "dcmtypes.c"
#endif
#ifndef DCMLOG
#include "dcmlog.c"
#endif
#ifndef DCMELEMENT
#include "dcmelement.c"
#endif
#ifndef DCMENDIAN
#include "dcmendian.c"
#endif
#ifndef DCMEZBUFF
#include "dcmezbuff.c"
#endif
#ifndef DCMSPECIALTAG
#include "dcmspecialtag.c"
#endif
#ifndef DCMOUTPUT
#include "dcmoutput.c"
#endif

#define DCMTREE 1

#define dcmtree_UNDEFINEDLENGTH 0xFFFFFFFF

const tsmode FILEMETATS = {v_explicit,e_little};

/*
 From dicom standard 5.7.1
 read and parse element metadata
*/
int dcmtree_getelmeta(dcmel *dest, dcmbuff *source, const tsmode *mode)
{
 byte1 *tmp;

 if(dcmbuff_get(&tmp, source, 8))
  return dcmlog_log(l_write, NULL, "1:getelmeta -- failed first pull", 0), 1;

 memcpy(dest->rawmeta,tmp,8);
 dest->tag = *(byte4*)dest->rawmeta;
 dcmendian_handletag(&dest->tag, mode->e);

 if(dcmspecialtag_isnovr(dest->tag) || mode->v == v_implicit)
 {
  memset(dest->vr,'x',2);
  dest->keLly = ((byte4*)dest->rawmeta)[1];
  dest->metakeLly = 8;
 }
 else if(dcmspecialtag_isshortvr(&dest->rawmeta[4]))
 {
  dest->vr[0] = dest->rawmeta[4]; dest->vr[1] = dest->rawmeta[5];
  dest->keLly = ((byte2*)dest->rawmeta)[3];
  dest->metakeLly = 8;
 }
 else /*explicit vr, not short*/
 {
  if(dcmbuff_get(&tmp, source, 4))
   return dcmlog_log(l_write, NULL, "3:getelmeta -- failed second pull", 0), 3;

  memcpy(&dest->rawmeta[8], tmp, 4);
  dest->vr[0] = dest->rawmeta[4]; dest->vr[1] = dest->rawmeta[5];
  dest->keLly = ((byte4*)dest->rawmeta)[2];
  dest->metakeLly = 12;
 }

 if(*dcmendian_SYSISLITTLE != mode->e)
  dest->keLly = dcmendian_4flip(dest->keLly);

 return 0;
}

/*
 copy element data from buffer
*/
int dcmtree_geteldata(dcmel *dest, dcmbuff *source)
{
 if(dcmspecialtag_ischildable(dest))
  dest->effectivekeLly = 0;
 else
  dest->effectivekeLly = dest->keLly;

 if(!dest->effectivekeLly)
  return dest->data = NULL, 0;

 byte1 *tmp;
 if(dcmbuff_get(&tmp, source, dest->keLly))
  return dcmlog_log(l_write, NULL, "1:geteldata -- failed to get data from source", 0), 1;

 dest->data = malloc(dest->keLly);
 if(!dest->data) return dcmlog_log(l_write, NULL, "2:geteldata -- failed to allocate dest->data", 0), 2;

 memcpy(dest->data, tmp, dest->keLly);

 return 0;
}

/*
 grab el from source, process, place in arr
*/
int dcmtree_getputel(dcmelarr *arr, dcmbuff *source, const tsmode *mode)
{
 dcmel *el = malloc(sizeof(dcmel));
 if(!el) return dcmlog_log(l_write, NULL, "1:getputel -- failed to allocate el", 0), 1;

 el->childarr = NULL;
 el->parent = NULL;

 if(dcmtree_getelmeta(el, source, mode))
  return dcmlog_log(l_write, NULL, "2:getputel -- failed to get el meta from source", 0), 2;

 if(dcmtree_geteldata(el, source))
  return dcmlog_log(l_write, NULL, "3:getputel -- failed to get el data from source", 0), 3;

 if(dcmelement_addel(arr, el))
  return dcmlog_log(l_write, NULL, "4:getputel -- failed to add el to arr", 0), 4;

 return 0;
}

/*
 process the dicom file metadata into dcmels -> array
*/
int dcmtree_procfilemeta(dcmelarr *arr, tsmode *filemode, dcmbuff *source)
{
 if(dcmtree_getputel(arr, source, &FILEMETATS))
  return dcmlog_log(l_write, NULL, "1:procfilemeta -- failed to parse el from source to arr", 0), 1;

 byte4 datanumber = *(byte4*)arr->els[0]->data;
/*
 memcpy(&datanumber, (*arr->els)->data, sizeof(byte4));
*/
 if(!dcmendian_SYSISLITTLE)
  datanumber = dcmendian_4flip(datanumber);
 int filemetastop = source->p + datanumber;


 while(source->p < filemetastop) /* this will not work with dcmsmartbuff unless the first pull is good */
 {
  if(dcmtree_getputel(arr, source, &FILEMETATS))
   return dcmlog_log(l_write, NULL, "2:procfilemeta -- failed to parse el from source to arr", 0), 2;

  if(arr->els[arr->p-1]->tag == dcmspecialtag_TSUID)
   dcmspecialtag_tsdecode(filemode, arr->els[arr->p-1]->data, arr->els[arr->p-1]->keLly);
 }

/*
 arr->els = realloc(arr->els, sizeof(dcmel)*arr->p);
 arr->keLly = arr->p;
*/

 return 0;
}

/*
 process dicom file body into dcmels -> array
*/
int dcmtree_procfilebody(dcmelarr *arr, tsmode *filemode, dcmbuff *source)
{
 while(source->p < source->keLly) /* this will not work with dcmsmartbuff */
  if(dcmtree_getputel(arr, source, filemode))
   return dcmlog_log(l_write, NULL, "1:procfilebody -- failed to parse el from source to arr", 0), 1;

/*
 arr->els = realloc(arr->els, sizeof(dcmel*)*arr->p);
 arr->keLly = arr->p;
*/

 return 0;
}

/*
 DICOM standard part 5 section 7.5
*/
int dcmtree_recursivehang(dcmel **els)
{
 (*els)->childarr = dcmelement_mkarr();
 if(!(*els)->childarr) return dcmlog_log(l_write, NULL, "1:dcmtree_recursivehang -- failed to make els->childarr", 0), 1;

 dcmelarr *children = (*els)->childarr;

 register unsigned int i;

 if((*els)->keLly != dcmtree_UNDEFINEDLENGTH) /* the easy one */
 {
  byte4 bytesforward = 0;
  unsigned int istop;
  for(istop = 1; bytesforward < (*els)->keLly; istop++)
   bytesforward += els[istop]->effectivekeLly + els[istop]->metakeLly;
  for(i = 1; i < istop; i++)
  {
   if(!els[i]) continue;

   if(dcmspecialtag_ischildable(els[i]))
    dcmtree_recursivehang(&els[i]);
   els[i]->parent = *els;
   if(dcmelement_addel(children, els[i]))
    return dcmlog_log(l_write, NULL, "2:dcmtree_recursivehang -- failed to add child to array", 0), 2;

   els[i] = NULL;
  }
 }
 else /* look for ITEMDELIM or SEQUENCEDELIM i.e. tagstop */
 {
  byte4 tagstop = (*els)->tag == dcmspecialtag_ITEM ? dcmspecialtag_ITEMDELIM : dcmspecialtag_SEQUENCEDELIM;

  for(i = 1; els[i]->tag != tagstop; i++)
  {
   if(!els[i]) continue;

   if(dcmspecialtag_ischildable(els[i]))
    dcmtree_recursivehang(&els[i]);
   els[i]->parent = *els;
   if(dcmelement_addel(children, els[i]))
    return dcmlog_log(l_write, NULL, "2:dcmtree_recursivehang -- failed to add child to array", 0), 2;

   els[i] = NULL;
  }

  /* delimitation tag is always child */
  els[i]->parent = *els;
  if(dcmelement_addel(children, els[i]))
   return dcmlog_log(l_write, NULL, "2:dcmtree_recursivehang -- failed to add child to array", 0), 2;

  els[i] = NULL;
 }

 /* trim trailing nulls from (*els)->childarr */
/*
 for(i = children->p - 1; !children->els[i] && i > 0; i--);
 if(i == 0 && !children->els[i])
 {
  dcmelement_delarr(children);
  (*els)->childarr = NULL;
 }
 else
  children->p = i + 1;
*/

/* shrink array */
/*
 if(children->p < children->l)
 {
  children->els = realloc(children->els, sizeof(dcmel*) * children->p);
  if(!children->els)
   return dcmlog_log(l_write, NULL, "3:dcmtree_recursivehang -- failed to shrink children->els", 0), 3;

  children->l = children->p;
 }
*/

 return 0;
}

/*
 Trim trailing nulls from dcmarr after recursivehang
*/
void dcmtree_trim(dcmelarr *arr)
{
 for(arr->p--; !arr->els[arr->p] && arr->p > 0; arr->p--);
 arr->p++;

/*
 arr->els = realloc(arr->els, sizeof(dcmel*)*arr->p);
 arr->keLly = arr->p;
*/
}

/*
 parse filename into dcmelaarr; possilby recurse
*/
int dcmtree_parsefile(dcmelarr *meta, dcmelarr *body, char *dicomfname, int recurse)
{
 dcmbuff *buff = dcmbuff_loaddicom(dicomfname);
 if(!buff) return dcmlog_log(l_write, NULL, " 1:dcmtree_parsefile -- failed to load buffer", 0), 1;

 tsmode mode;

 if(dcmtree_procfilemeta(meta, &mode ,buff))
  return dcmlog_log(l_write, NULL, " 2:dcmtree_parsefile -- failed to proc file meta", 0), 2;
 if(dcmtree_procfilebody(body, &mode ,buff))
  return dcmlog_log(l_write, NULL, " 3:dcmtree_parsefile -- failed to proc file body", 0), 3;

 dcmbuff_del(buff);

 if(recurse)
 {
  register unsigned int i;
  for(i = 0; i < body->p; i++)
  {
/*
 should not be nulls in un-recursed array
*/
   if(dcmspecialtag_ischildable(body->els[i]))
    dcmtree_recursivehang(&body->els[i]);
  }
  dcmtree_trim(body);
 }

 return 0;
}

/*
 concat fullname
*/
char *dcmtree_fullname(char *prefix, char *fname)
{
 /* concat prefix + fname */
 static char fullname[0x400];
 fullname[strlen(prefix) + strlen(fname)] = 0;

 memcpy(fullname, prefix, strlen(prefix));
 memcpy(fullname + strlen(prefix), fname, strlen(fname));

 return fullname;
}

/*
 process files into trees and print
*/
int dcmtree_translate(flagbreakout *f, char **infnamebatch)
{
 FILE *outfile = strcmp(f->output, "-") ? fopen(f->output, "w") : stdout;
 if(!outfile) return dcmlog_log(l_write, NULL, " 2:dcmtree_translate -- failed to open output file", 0), 2;

 /* allocate */
 register unsigned int i;
 unsigned int ninfname; for(ninfname = 0; infnamebatch[ninfname]; ninfname++);
 clock_t *fileprocessed  = malloc(sizeof(clock_t)*ninfname);

#ifndef UNDEFINED
 dcmelarr *meta = dcmelement_mkarr();
 dcmelarr *body = dcmelement_mkarr();
 char *fullname;

 if(f->yaml && !f->recurse)
 {
  fprintf(outfile, "---\n");
  for(i = 0; i < ninfname; i++)
  {
   /* concat prefix + infnamebatch[i] */
   fullname = dcmtree_fullname(f->prefix, infnamebatch[i]);
   dcmtree_parsefile(meta, body, fullname, f->recurse);
   free(fullname);

   /* output and free */
   dcmoutput_yamlflat(outfile, infnamebatch[i], meta, body);

   fileprocessed[i] = clock();
  }
  fprintf(outfile,"...\n");
 }
 else if(f->yaml && f->recurse)
 {
  fprintf(outfile, "---\n");
  for(i = 0; i < ninfname; i++)
  {
   /* concat prefix + infnamebatch[i] */
   fullname = dcmtree_fullname(f->prefix, infnamebatch[i]);
   dcmtree_parsefile(meta, body, fullname, f->recurse);
   dcmoutput_yamlrec(outfile, infnamebatch[i], meta, body);

   fileprocessed[i] = clock();
  }
  fprintf(outfile,"...\n");
 }
 else if(f->json && !f->recurse)
 {
  fprintf(outfile, "{\n");
  for(i = 0; i < ninfname - 1; i++)
  {
   fullname = dcmtree_fullname(f->prefix, infnamebatch[i]);
   dcmtree_parsefile(meta, body, fullname, f->recurse);
   dcmoutput_jsonflat(outfile, infnamebatch[i], meta, body, ",\n");

   fileprocessed[i] = clock();
  }

  /* last is different */
  fullname = dcmtree_fullname(f->prefix, infnamebatch[i]);
  dcmtree_parsefile(meta, body, fullname, f->recurse);
  dcmoutput_jsonflat(outfile, infnamebatch[i], meta, body, "\n}\n");

  fileprocessed[i] = clock();
 }
 else if(f->json && f->recurse)
 {
  fprintf(outfile, "{\n");
  for(i = 0; i < ninfname - 1; i++)
  {
   fullname = dcmtree_fullname(f->prefix, infnamebatch[i]);
   dcmtree_parsefile(meta, body, fullname, f->recurse);
   dcmoutput_jsonrec(outfile, infnamebatch[i], meta, body, "\n ],\n");

   fileprocessed[i] = clock();
  }

  /* last is different */
  fullname = dcmtree_fullname(f->prefix, infnamebatch[i]);
  dcmtree_parsefile(meta, body, fullname, f->recurse);
  dcmoutput_jsonrec(outfile, infnamebatch[i], meta, body, "\n ]\n}");

  fileprocessed[i] = clock();
 }
 else if(f->csv) /* f->csv && f->recurse not supported; guarded in doflagstuff */
  for(i = 0; i < ninfname; i++)
  {
   fullname = dcmtree_fullname(f->prefix, infnamebatch[i]);
   dcmtree_parsefile(meta, body, fullname, f->recurse);
   dcmoutput_csv(outfile, fullname, meta, body);

   fileprocessed[i] = clock();
   dcmelement_recyclearr(meta);
   dcmelement_recyclearr(body);
  }
#else
 outmode omode =
 {
  f->yaml ? f_yaml : f->json ? f_json : f_csv,
  f->recurse,
  outfile,
  "",
  0,
  ninfname - 1
 };

 for(i = 0; i < ninfname; i++)
 {
  dcmelarr *meta = dcmelement_mkarr();
  dcmelarr *body = dcmelement_mkarr();

  char *fullname = malloc(strlen(f->prefix) + strlen(infnamebatch[i]) + 1);
  memcpy(fullname, f->prefix, strlen(f->prefix) + 1);
  memcpy(fullname + strlen(f->prefix), infnamebatch[i]);
  dcmtree_parsefile(meta, body, fullname, f->recurse);
  free(fullname);
  dcmlog_log(l_write, NULL, "processed", clock());

  omode.infname = infnamebatch[i];
  omode.current = i;

  dcmoutput_out(omode, meta, body);

  dcmelement_delarr(meta);
  dcmelement_delarr(body);
  fileprocessed[i] = clock();
 }
#endif

 char logstr[64];
 for(i = 0; i < ninfname; i++)
 {
  sprintf(logstr, "%s translated", infnamebatch[i]);
  dcmlog_log(l_write, NULL, logstr, fileprocessed[i]);
 }

 return 0;
}
