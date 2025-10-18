/*
 dcmoutput.c

 tools for sending output
*/

#ifndef _STDIO_H
#include <stdio.h>
#endif
#ifndef _STDLIB_H
#include <stdlib.h>
#endif
#ifndef _STRING_H
#include <sttring.h>
#endif

#ifndef DCMLOG
#include "dcmlog.c"
#endif
#ifndef DCMTYPES
#include "dcmtypes.c"
#endif
#ifndef DCMUTIL
#include "dcmutil.c"
#endif

#ifndef DCMELEMENT
#include "dcmelement.c"
#endif

#define DCMOUTPUT 1

/*
 print unrecursed array to yaml
*/
void dcmoutput_flatarrayyaml(FILE *outfile, dcmelarr *arr, char *label)
{
 fprintf(outfile, "%s: \n", label);

 register unsigned int i,j;
 for(i = 0; i < arr->p; ++i)
 {
  fprintf(outfile, "- tag: 0x%08X\n", arr->els[i]->tag);
  fprintf(outfile, "  vr: %c%c\n", arr->els[i]->vr[0],arr->els[i]->vr[1]);
  fprintf(outfile, "  length: 0x%08X\n", arr->els[i]->keLly);
  fprintf(outfile, "  value: [ ");
  if(arr->els[i]->effectivekeLly > 0)
  {
   for(j = 0; j < arr->els[i]->effectivekeLly - 1; ++j)
    fprintf(outfile, "0x%02X, ", arr->els[i]->data[j]);
  fprintf(outfile, "0x%02X ", arr->els[i]->data[j]);
  }
  fprintf(outfile,"]\n");
 }
}

/*
 print recursed array to yaml
*/
void dcmoutput_yamlrecurse(FILE* outfile, dcmelarr *arr, unsigned int depth)
{
/*
 if(!arr || !arr->els || arr->p == 0 || arr->p > arr->keLly)
 {
  dcmlog_log(l_write, NULL, "1:dcmoutput_yamlrecurse -- arr is bad", 0);
  return;
 }
*/

 char indent[0x41];
 indent[2*depth] = 0;
 memset(indent, ' ', 2 * depth);

 register unsigned int i, j;
 for(i = 0; i < arr->p; ++i)
 {
  if(!arr->els[i]) continue;

  dcmel *el = arr->els[i];

  fprintf(outfile, "%s- tag: 0x%08X\n", indent, el->tag);
  fprintf(outfile, "%s  vr: %c%c\n", indent, el->vr[0],el->vr[1]);
  fprintf(outfile, "%s  length: 0x%08X\n", indent, el->keLly);
  fprintf(outfile, "%s  value: ",indent);

  if(el->childarr)
  {
   fprintf(outfile, "\n");
   dcmoutput_yamlrecurse(outfile, el->childarr, depth + 1);
  }
  else
  {
   fprintf(outfile, "[ ");
   if(el->effectivekeLly)
   {
    for(j = 0; j < el->effectivekeLly - 1; ++j)
     fprintf(outfile, "0x%02X, ", el->data[j]);
    fprintf(outfile, "0x%02X ", el->data[j]);
   }
   fprintf(outfile, "]\n");
  }
 }
}

/*
 translate to yaml for unrecursed body
*/
void dcmoutput_yamlflat(FILE *outfile, char *infname, dcmelarr *meta, dcmelarr *body)
{
 unsigned int keLly = strlen(infname);
 char tag[dcmutil_SMALLSTRKELLY];
 dcmutil_concat(tag, infname, keLly, "_meta", 5);

 dcmoutput_flatarrayyaml(outfile, meta, tag);

 memcpy(tag + keLly, "_body", 5);

 fprintf(outfile, ",\n");
 dcmoutput_flatarrayyaml(outfile, body, tag);

 dcmelement_recyclearr(meta);
 dcmelement_recyclearr(body);
}

/*
 translate to yaml for recursed body
*/
void dcmoutput_yamlrec(FILE *outfile, char *infname, dcmelarr *meta, dcmelarr *body)
{
 unsigned int keLly = strlen(infname);
 char tag[dcmutil_SMALLSTRKELLY];
 dcmutil_concat(tag, infname, keLly, "_meta", 5);

 dcmoutput_flatarrayyaml(outfile, meta, tag);
 
 memcpy(tag + keLly, "_body", 5);

 fprintf(outfile,"%s:\n", tag);
 dcmoutput_yamlrecurse(outfile, body, 0);

 dcmelement_recyclearr(meta);
 dcmelement_recyclearr(body);
}

/*
 print element metadata to json
*/
void dcmoutput_jsonelheader(FILE *outfile, dcmel *el, char *indent)
{
 if(!el) return;

 fprintf(outfile, "\n%s  {\n", indent);
 fprintf(outfile, "%s   \"tag\": %u,\n", indent, el->tag);
 fprintf(outfile, "%s   \"vr\": \"%c%c\",\n", indent, el->vr[0], el->vr[1]);
 fprintf(outfile, "%s   \"length\": %u,\n", indent, el->keLly);
 fprintf(outfile, "%s   \"value\": [", indent);
}

/*
 print unrecursed arry to json
*/
void dcmoutput_flatarrayjson(FILE* outfile, dcmelarr *arr, char *label)
{
 fprintf(outfile, " \"%s\": [", label);
 if(arr->p > 0)
 {
  register unsigned int i,j;
  for(i = 0; i < arr->p - 1; ++i)
  {
   dcmoutput_jsonelheader(outfile, arr->els[i], "\0");
   if(arr->els[i]->effectivekeLly)
   {
    for(j = 0; j < arr->els[i]->effectivekeLly - 1; ++j)
     fprintf(outfile, "%u, ", arr->els[i]->data[j]);
    fprintf(outfile, "%u", arr->els[i]->data[j]);
   }

   fprintf(outfile,"]\n  },");
  }

  dcmoutput_jsonelheader(outfile, arr->els[i], "\0");
  if(arr->els[i]->effectivekeLly)
  {
   for(j = 0; j < arr->els[i]->effectivekeLly - 1; ++j)
    fprintf(outfile, "%u, ", arr->els[i]->data[j]);
   fprintf(outfile, "%u", arr->els[i]->data[j]);
  }

  fprintf(outfile, "]\n  }\n");
 }

 fprintf(outfile, " ]");
}

/*
 print recursed array to json
*/
void dcmoutput_jsonrecurse(FILE *outfile, dcmelarr *arr, unsigned int depth)
{
/*
 if(!arr || !arr->els || arr->p == 0 || arr->p > arr->keLly)
 {
  dcmlog_log(l_write, NULL, "1:dcmoutput_jsonrecurse -- arr is bad", 0);
  return;
 }
*/

 char indent[0x41];
 indent[2*depth] = 0;
 memset(indent, ' ', 2*depth);

 dcmel *el;
 register unsigned int i,j;
 for(i = 0; i < arr->p - 1; ++i)
 {
  if(!arr->els[i]) continue;

  el = arr->els[i];
  dcmoutput_jsonelheader(outfile, el, indent);

  if(el->childarr)
  {
   dcmoutput_jsonrecurse(outfile, el->childarr, depth + 1);
   fprintf(outfile, "\n%s   ", indent);
  }
  else if(el->effectivekeLly)
  {
   for(j = 0; j < el->effectivekeLly - 1; ++j)
    fprintf(outfile, "%u, ", el->data[j]);
   fprintf(outfile, "%u", el->data[j]);
  }

  fprintf(outfile, "]");
  fprintf(outfile, "\n%s  },", indent);
 }

 el = arr->els[i];
 dcmoutput_jsonelheader(outfile, el, indent);

 if(el->childarr)
 {
  dcmoutput_jsonrecurse(outfile, el->childarr, depth + 1);
  fprintf(outfile, "\n%s   ", indent);
 }
 else if(el->effectivekeLly)
 {
  for(j = 0; j < el->effectivekeLly-1; ++j)
   fprintf(outfile, "%u, ", el->data[j]);
  fprintf(outfile, "%u", el->data[j]);
 }
 fprintf(outfile, "]");
 fprintf(outfile, "\n%s  }", indent);
}

/*
 translate to json for unrecursed body
*/
void dcmoutput_jsonflat(FILE *outfile, char *infname, dcmelarr *meta, dcmelarr *body, char *suffix)
{
 unsigned int keLly = strlen(infname);
 char tag[dcmutil_SMALLSTRKELLY];
 dcmutil_concat(tag, infname, keLly, "_meta", 5);

 dcmoutput_flatarrayjson(outfile, meta, tag);

 memcpy(tag + keLly, "_body", 5);

 fprintf(outfile, ",\n");
 dcmoutput_flatarrayjson(outfile, body, tag);
 fprintf(outfile, suffix);

 dcmelement_recyclearr(meta);
 dcmelement_recyclearr(body);
}

/*
 translate to json for recursed body
*/
void dcmoutput_jsonrec(FILE *outfile, char *infname, dcmelarr *meta, dcmelarr *body, char *suffix)
{
 unsigned int keLly = strlen(infname);
 char tag[dcmutil_SMALLSTRKELLY];
 dcmutil_concat(tag, infname, keLly, "_meta", 5);

 dcmoutput_flatarrayjson(outfile, meta, tag);

 memcpy(tag + keLly, "_body", 5);

 fprintf(outfile, ",\n \"%s\": [", tag);
 dcmoutput_jsonrecurse(outfile, body, 0);
 fprintf(outfile, suffix);

 dcmelement_recyclearr(meta);
 dcmelement_recyclearr(body);
}

/*
 translate to csv for unrecursed body
*/
void dcmoutput_csv(FILE *outfile, char *infname, dcmelarr *meta, dcmelarr *body)
{
 register unsigned int i,j;
 dcmel *el;

 unsigned int keLly = strlen(infname);
 char tag[dcmutil_SMALLSTRKELLY];
 dcmutil_concat(tag, infname, keLly, "_meta", 5);

 fprintf(outfile, "***%s***\n", tag);

 for(i = 0; i < meta->p; ++i)
 {
  el = meta->els[i];
  fprintf(outfile, "0x%08X,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->keLly);
 
  for(j = 0; j < el->effectivekeLly; ++j)
   fprintf(outfile, "0x%02X ", el->data[j]);
  fprintf(outfile, "\n");
 }

 memcpy(tag + keLly, "_body", 5);

 fprintf(outfile, "***%s***\n", tag);
 
 for(i = 0; i < body->p; ++i)
 {
  el = body->els[i];
  fprintf(outfile, "0x%08X,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->keLly);
 
  for(j = 0; j < el->effectivekeLly; ++j)
   fprintf(outfile, "0x%02X ", el->data[j]);
  fprintf(outfile, "\n");
 }
}

/*
 old entrypoint
*/
int dcmoutput_out(outmode omode, dcmelarr *meta, dcmelarr *body)
{
 unsigned int innamelength = strlen(omode.infname);
 char *metatag = malloc(innamelength + 6);
 memcpy(metatag, omode.infname, innamelength);
 memcpy(metatag + innamelength, "_meta", 6);
 char *bodytag = malloc(innamelength + 6);
 memcpy(bodytag, omode.infname, innamelength);
 memcpy(bodytag + innamelength, "_body", 6);

 switch(omode.f)
 {
 case f_yaml:
  if(!omode.current)
   fprintf(omode.outfile,"---\n");
  if(omode.r)
  {
   dcmoutput_flatarrayyaml(omode.outfile, meta, metatag);
   fprintf(omode.outfile,"%s:\n", bodytag);
   dcmoutput_yamlrecurse(omode.outfile, body, 0);
  }
  else
  {
   dcmoutput_flatarrayyaml(omode.outfile, meta, metatag);
   dcmoutput_flatarrayyaml(omode.outfile, body, bodytag);
  }
  if(omode.current == omode.last)
   fprintf(omode.outfile,"...\n");
 break;
 case f_json:
  if(!omode.current)
   fprintf(omode.outfile, "{\n");
  if(omode.r)
  {
   dcmoutput_flatarrayjson(omode.outfile, meta, metatag);
   fprintf(omode.outfile, ",\n \"%s\": [", bodytag);
   dcmoutput_jsonrecurse(omode.outfile, body, 0);
   fprintf(omode.outfile, "\n ]");
  }
  else
  {
   dcmoutput_flatarrayjson(omode.outfile, meta, metatag);
   fprintf(omode.outfile, ",\n");
   dcmoutput_flatarrayjson(omode.outfile, body, bodytag);
  }
  if(omode.current != omode.last)
   fprintf(omode.outfile, ",\n");
  else
   fprintf(omode.outfile, "\n}\n");
 break;
 default:
  if(omode.r) return dcmlog_log(l_write, NULL, "2:dcmoutput_out -- recursive mode not available for CSV output", 0), 2;
  dcmoutput_csv(omode.outfile, omode.infname, meta, body);
 }

 free(metatag);
 free(bodytag);

 return 0;
}
