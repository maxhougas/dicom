/*
 dcmoutput.c

 tools for sending output
*/

#ifndef _DCMTYPES
#include "dcmtypes.c"
#endif
#ifndef _DCMELEMENT
#include "dcmelement.c"
#endif

#define _DCMOUTPUT 1

int dcmoutput_flatarrayyaml(FILE *outfile, dcmelarr *arr, char* label)
{
 unsigned int i,j;

 fprintf(outfile, "%s: \n", label);
 for(i = 0; i < arr->p; i++)
 {
  fprintf(outfile, "- tag: 0x%08X\n", arr->els[i]->tag);
  fprintf(outfile, "  vr: %c%c\n", arr->els[i]->vr[0],arr->els[i]->vr[1]);
  fprintf(outfile, "  length: 0x%08X\n", arr->els[i]->length);
  fprintf(outfile, "  value: [ ");
  if(arr->els[i]->effectivelength > 0)
  {
   for(j = 0; j < arr->els[i]->effectivelength - 1; j++)
    fprintf(outfile, "0x%02X, ", arr->els[i]->data[j]);
  fprintf(outfile, "0x%02X ", arr->els[i]->data[j]);
  }
  fprintf(outfile,"]\n");
 }
}

/*
int dcmoutput_yaml(FILE *outfile, dcmelarr *meta, dcmelarr *body)
{
 unsigned int i,j;
 dcmel *el;

 fprintf(outfile, "---\n");

 fprintf(outfile, "meta: \n");
 for(i = 0; i < meta->p; i++)
 {
  el = meta->els[i];
  fprintf(outfile, "- tag: 0x%08X\n", el->tag);
  fprintf(outfile, "  vr: %c%c\n", el->vr[0],el->vr[1]);
  fprintf(outfile, "  length: 0x%08X\n", el->length);
  fprintf(outfile, "  value: [ ");
  if(el->length > 0)
  {
   for(j = 0; j < el->length - 1; j++)
    fprintf(outfile, "0x%02X, ", el->data[j]);
  fprintf(outfile, "0x%02X ", el->data[j]);
  }
  fprintf(outfile,"]\n");
 }

 fprintf(outfile, "body: \n");
 for(i = 0; i < body->p; i++)
 {
  el = body->els[i];
  fprintf(outfile, "- tag: 0x%08X\n", el->tag);
  fprintf(outfile, "  vr: %c%c\n", el->vr[0],el->vr[1]);
  fprintf(outfile, "  length: 0x%08X\n", el->length);
  fprintf(outfile, "  value: [ ");
  if(el->length > 0)
  {
   for(j = 0; j < el->length - 1; j++)
    fprintf(outfile, "0x%02X, ", el->data[j]);
   fprintf(outfile,"0x%02X ", el->data[j]); 
  }
  fprintf(outfile,"]\n");
 }

 fprintf(outfile, "...\n");
}

int dcmoutput_yamlmeta(FILE *outfile, dcmelarr *meta)
{
 unsigned int i,j;
 dcmel *el;

 fprintf(outfile, "meta: \n");
 for(i = 0; i < meta->p; i++)
 {
  el = meta->els[i];
  fprintf(outfile, "- tag: 0x%08X\n", el->tag);
  fprintf(outfile, "  vr: %c%c\n", el->vr[0],el->vr[1]);
  fprintf(outfile, "  length: 0x%08X\n", el->length);
  fprintf(outfile, "  value: [ ");
  if(el->length > 0)
  {
   for(j = 0; j < el->length - 1; j++)
    fprintf(outfile, "0x%02X, ", el->data[j]);
  fprintf(outfile, "0x%02X ", el->data[j]);
  }
  fprintf(outfile,"]\n");
 }
}
*/

void dcmoutput_yamlrecurse(FILE* outfile, dcmel *el, unsigned int depth)
{
 if(el == NULL) return;

 char *indent = (char*)malloc(2*depth + 1);
 indent[2*depth] = 0;
 memset(indent, ' ', 2 * depth);

 fprintf(outfile, "%s- tag: 0x:%08X\n", indent, el->tag);
 fprintf(outfile, "%s  vr: %c%c\n", indent, el->vr[0],el->vr[1]);
 fprintf(outfile, "%s  length: 0x%08X\n", indent, el->length);
 fprintf(outfile, "%s  value: ",indent);

 free(indent);

 unsigned int i;
 if(el->nchildren)
 {
  fprintf(outfile, "\n");
  for(i = 0; i < el->nchildren; i++)
   dcmoutput_yamlrecurse(outfile, el->children[i], depth + 1);
 }
 else
 {
  fprintf(outfile, "[ ");
  if(el->effectivelength > 0)
  {
   for(i = 0; i < el->effectivelength - 1; i++)
    fprintf(outfile, "%02X, ", el->data[i]);
   fprintf(outfile, "%02X ", el->data[i]);
  }
  fprintf(outfile, "]\n");
 }
}

void dcmoutput_jsonel(FILE* outfile, dcmel *el)
{
 fprintf(outfile, "  {\n");
 fprintf(outfile, "   \"tag\": %u,\n", el->tag);
 fprintf(outfile, "   \"vr\": \"%c%c\",\n", el->vr[0], el->vr[1]);
 fprintf(outfile, "   \"length\": %u,\n", el->effectivelength);
 fprintf(outfile, "   \"value\": [");

 if(el->effectivelength > 0)
 {
  unsigned int i;
  for(i = 0; i < el->effectivelength - 1; i++)
   fprintf(outfile, "%u, ", el->data[i]);
  fprintf(outfile, "%u", el->data[i]);
 }
 fprintf(outfile,"]\n");
 fprintf(outfile, "  }");
}

void dcmoutput_flatarrayjson(FILE* outfile, dcmelarr *arr, char* label)
{
 fprintf(outfile, " \"%s\": [\n", label);
 if(arr->p > 0)
 {
  unsigned int i;
  for(i = 0; i < arr->p - 1; i++)
  {
   dcmoutput_jsonel(outfile, arr->els[i]);
   fprintf(outfile, ",\n");
  }
  dcmoutput_jsonel(outfile, arr->els[i]);
  fprintf(outfile, "\n");
 }
 fprintf(outfile, " ]");
}

/*
int dcmoutput_json(FILE *outfile, dcmelarr *meta, dcmelarr *body)
{
 unsigned int i;
 dcmel *el;

 fprintf(outfile, "{\n");

 fprintf(outfile, " \"meta\": [\n");
 if(meta->p > 0)
 {
  for(i = 0; i < meta->p - 1; i++)
  {
   el = meta->els[i];
   dcmoutput_jsonel(outfile, el);
   fprintf(outfile, ",\n");
  }
  el = meta->els[i];
  dcmoutput_jsonel(outfile, el);
  fprintf(outfile, "\n");
 }
 fprintf(outfile, " ],\n");

 fprintf(outfile, " \"body\": [\n");
 if(body->p > 0)
 {
  for(i = 0; i < body->p - 1; i++)
  {
   el = body->els[i];
   dcmoutput_jsonel(outfile, el);
   fprintf(outfile,",\n");
  }
  el = body->els[i];
  dcmoutput_jsonel(outfile, el);
  fprintf(outfile,"\n");
 }
 fprintf(outfile, " ]\n");
 
 fprintf(outfile, "}");
}

void dcmoutput_jsonmeta(FILE* outfile, dcmelarr *meta)
{
 unsigned int i;
 dcmel *el;

 fprintf(outfile, " \"meta\": [\n");
 if(meta->p > 0)
 {
  for(i = 0; i < meta->p - 1; i++)
  {
   el = meta->els[i];
   dcmoutput_jsonel(outfile, el);
   fprintf(outfile, ",\n");
  }
  el = meta->els[i];
  dcmoutput_jsonel(outfile, el);
  fprintf(outfile, "\n");
 }
 fprintf(outfile, " ],\n");
}
*/

void dcmoutput_jsonrecurse(FILE *outfile, dcmel* el, unsigned int depth)
{
 if(el == NULL) return;

 char *indent = (char*)malloc(2*depth+1);
 indent[2*depth] = 0;
 memset(indent, ' ', 2*depth);

 fprintf(outfile, "\n%s  {\n", indent);
 fprintf(outfile, "%s   \"tag\": %u,\n", indent, el->tag);
 fprintf(outfile, "%s   \"vr\": \"%c%c\",\n", indent, el->vr[0], el->vr[1]);
 fprintf(outfile, "%s   \"length\": %u,\n", indent, el->length);
 fprintf(outfile, "%s   \"value\": [", indent);

 unsigned int i;
 if(el->nchildren)
 {
  for(i = 0; i < el->nchildren - 1; i++)
  {
   dcmoutput_jsonrecurse(outfile, el->children[i], depth + 1);
   fprintf(outfile, ",", indent, indent);
  }
  dcmoutput_jsonrecurse(outfile, el->children[i], depth + 1);
  fprintf(outfile, "\n%s   ]", indent);
  fprintf(outfile, "\n%s  }", indent);
 }
 else
 {
  if(el->effectivelength > 0)
  {
   for(i = 0; i < el->effectivelength - 1; i++)
    fprintf(outfile, "%u, ", el->data[i]);
   fprintf(outfile, "%u", el->data[i]);
  }
  fprintf(outfile, "]");
  fprintf(outfile, "\n%s  }", indent);
 }

 free(indent);
}

int dcmoutput_csv(FILE *outfile, dcmelarr *meta, dcmelarr *body)
{
 int i,j;
 dcmel *el;

 fprintf(outfile, "***METASTART***\n");

 for(i = 0; i < meta->p; i++)
 {
  el = meta->els[i];
  fprintf(outfile, "0x%08X,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);
 
  for(j = 0; j < el->effectivelength; j++)
   fprintf(outfile, "0x%02X ", el->data[j]);
  fprintf(outfile, "\n");
 }

 fprintf(outfile, "***BODYSTART***\n");
 
 for(i = 0; i < body->p; i++)
 {
  el = body->els[i];
  fprintf(outfile, "0x%08X,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);
 
  for(j = 0; j < el->effectivelength; j++)
   fprintf(outfile, "0x%02X ", el->data[j]);
  fprintf(outfile, "\n");
 }
}

int dcmoutput_out(outmode omode, dcmelarr *meta, dcmelarr *body)
{
 int i,j;
 dcmel *el;
 FILE *outfile = strcmp("-",omode.outfname) ? fopen(omode.outfname, "w") : stdout;
 if(outfile == NULL) return perror("1:dcmoutput_out"), 1;

 switch(omode.f)
 {
 case f_yaml:
  if(omode.r)
  {
   fprintf(outfile,"---\n");
   dcmoutput_flatarrayyaml(outfile, meta, "meta");
   fprintf(outfile,"body:\n");
   for(i = 0; i < body->p; i++)
    if(body->els[i] != NULL)
     dcmoutput_yamlrecurse(outfile, body->els[i], 0);
   fprintf(outfile,"...\n");
  }
  else
  {
   fprintf(outfile,"---\n");
   dcmoutput_flatarrayyaml(outfile, meta, "meta");
   dcmoutput_flatarrayyaml(outfile, body, "body");
   fprintf(outfile,"...\n");
  }
 break;
 case f_json:
  if(omode.r)
  {
   fprintf(outfile, "{\n");
   dcmoutput_flatarrayjson(outfile, meta, "meta");
   fprintf(outfile, ",\n \"body\": [");
   for(i = 0; i < body->p - 1; i++)
    if(body->els[i] != NULL)
    {
     dcmoutput_jsonrecurse(outfile, body->els[i], 0);
     fprintf(outfile, ",");
    }
   if(body->els[i] != NULL)
    dcmoutput_jsonrecurse(outfile, body->els[i], 0);
   fprintf(outfile, "\n ]\n}\n");
  }
  else
  {
   fprintf(outfile, "{\n");
   dcmoutput_flatarrayjson(outfile, meta, "meta");
   fprintf(outfile, ",\n");
   dcmoutput_flatarrayjson(outfile, body, "body");
   fprintf(outfile, "\n}\n");
  }
 break;
 default:
  if(omode.r) return perror("2:dcmoutput_out"), 2;
  dcmoutput_csv(outfile, meta, body);
 }

 int err = outfile == stdin ? 0 : fclose(outfile);
 if(err) return perror("3:dcmoutput_out"), 3;

 return 0;
}
