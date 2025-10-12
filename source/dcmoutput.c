/*
 dcmoutput.c

 tools for sending output
*/

#ifndef DCMTYPES
#include "dcmtypes.c"
#endif
#ifndef DCMELEMENT
#include "dcmelement.c"
#endif

#define DCMOUTPUT 1

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

 return 0;
}

int dcmoutput_yamlrecurse(FILE* outfile, dcmelarr *arr, unsigned int depth)
{
 if(arr == NULL || arr->els == NULL || arr->p == 0 || arr->p > arr->l) return perror("1:dcmoutput_yamlrecurse -- arr is bad"), 1;

 char *indent = malloc(2*depth + 1);
 indent[2*depth] = 0;
 memset(indent, ' ', 2 * depth);

 unsigned int i;
 for(i = 0; i < arr->p; i++)
 {
  if(arr->els[i] == NULL) continue;

  dcmel *el = arr->els[i];

  fprintf(outfile, "%s- tag: 0x%08X\n", indent, el->tag);
  fprintf(outfile, "%s  vr: %c%c\n", indent, el->vr[0],el->vr[1]);
  fprintf(outfile, "%s  length: 0x%08X\n", indent, el->length);
  fprintf(outfile, "%s  value: ",indent);

  if(el->childarr != NULL)
  {
   fprintf(outfile, "\n");
   dcmoutput_yamlrecurse(outfile, el->childarr, depth + 1);
  }
  else
  {
   fprintf(outfile, "[ ");
   if(el->effectivelength)
   {
    unsigned int j;
    for(j = 0; j < el->effectivelength - 1; j++)
     fprintf(outfile, "0x%02X, ", el->data[j]);
    fprintf(outfile, "0x%02X ", el->data[j]);
   }
   fprintf(outfile, "]\n");
  }
 }

 free(indent);

 return 0;
}

void dcmoutput_jsonelheader(FILE *outfile, dcmel *el, char *indent)
{
 if(el == NULL) return;

 fprintf(outfile, "\n%s  {\n", indent);
 fprintf(outfile, "%s   \"tag\": %u,\n", indent, el->tag);
 fprintf(outfile, "%s   \"vr\": \"%c%c\",\n", indent, el->vr[0], el->vr[1]);
 fprintf(outfile, "%s   \"length\": %u,\n", indent, el->length);
 fprintf(outfile, "%s   \"value\": [", indent);
}

void dcmoutput_flatarrayjson(FILE* outfile, dcmelarr *arr, char* label)
{
 fprintf(outfile, " \"%s\": [", label);
 if(arr->p > 0)
 {
  unsigned int i,j;
  for(i = 0; i < arr->p - 1; i++)
  {
   dcmoutput_jsonelheader(outfile, arr->els[i], "\0");
   if(arr->els[i]->effectivelength)
   {
    for(j = 0; j < arr->els[i]->effectivelength - 1; j++)
     fprintf(outfile, "%u, ", arr->els[i]->data[j]);
    fprintf(outfile, "%u", arr->els[i]->data[j]);
   }

   fprintf(outfile,"]\n  },");
  }

  dcmoutput_jsonelheader(outfile, arr->els[i], "\0");
  if(arr->els[i]->effectivelength)
  {
   for(j = 0; j < arr->els[i]->effectivelength - 1; j++)
    fprintf(outfile, "%u, ", arr->els[i]->data[j]);
   fprintf(outfile, "%u", arr->els[i]->data[j]);
  }

  fprintf(outfile, "]\n  }\n");
 }

 fprintf(outfile, " ]");
}

int dcmoutput_jsonrecurse(FILE *outfile, dcmelarr *arr, unsigned int depth)
{
 if(arr == NULL || arr->els == NULL || arr->p == 0 || arr->p > arr->l) return perror("1:dcmoutput_jsonrecurse -- arr is bad"), 1;

 char *indent = malloc(2*depth+1);
 indent[2*depth] = 0;
 memset(indent, ' ', 2*depth);

 dcmel *el;
 unsigned int i,j;
 for(i = 0; i < arr->p - 1; i++)
 {
  if(arr->els[i] == NULL) continue;

  el = arr->els[i];
  dcmoutput_jsonelheader(outfile, el, indent);

  if(el->childarr != NULL)
  {
   dcmoutput_jsonrecurse(outfile, el->childarr, depth + 1);
   fprintf(outfile, "\n%s   ", indent);
  }
  else if(el->effectivelength)
  {
   for(j = 0; j < el->effectivelength - 1; j++)
    fprintf(outfile, "%u, ", el->data[j]);
   fprintf(outfile, "%u", el->data[j]);
  }

  fprintf(outfile, "]");
  fprintf(outfile, "\n%s  },", indent);
 }

 el = arr->els[i];
 dcmoutput_jsonelheader(outfile, el, indent);

 if(el->childarr != NULL)
 {
  dcmoutput_jsonrecurse(outfile, el->childarr, depth + 1);
  fprintf(outfile, "\n%s   ", indent);
 }
 else if(el->effectivelength)
 {
  for(j = 0; j < el->effectivelength-1; j++)
   fprintf(outfile, "%u, ", el->data[j]);
  fprintf(outfile, "%u", el->data[j]);
 }
 fprintf(outfile, "]");
 fprintf(outfile, "\n%s  }", indent);

 free(indent);

 return 0;
}

int dcmoutput_csv(FILE *outfile, dcmelarr *meta, char *metatag, dcmelarr *body, char *bodytag)
{
 int i,j;
 dcmel *el;

 fprintf(outfile, "***%s***\n", metatag);

 for(i = 0; i < meta->p; i++)
 {
  el = meta->els[i];
  fprintf(outfile, "0x%08X,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);
 
  for(j = 0; j < el->effectivelength; j++)
   fprintf(outfile, "0x%02X ", el->data[j]);
  fprintf(outfile, "\n");
 }

 fprintf(outfile, "***%s***\n", bodytag);
 
 for(i = 0; i < body->p; i++)
 {
  el = body->els[i];
  fprintf(outfile, "0x%08X,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);
 
  for(j = 0; j < el->effectivelength; j++)
   fprintf(outfile, "0x%02X ", el->data[j]);
  fprintf(outfile, "\n");
 }

 return 0;
}

int dcmoutput_out(outmode omode, dcmelarr *meta, dcmelarr *body)
{
 unsigned int innamelength = strlen(omode.tag) + 1;
 char *metatag = malloc(innamelength+5);
 memcpy(metatag, omode.tag, innamelength);
 strcat(metatag, "_meta");
 char *bodytag = malloc(innamelength+5);
 memcpy(bodytag, omode.tag, innamelength);
 strcat(bodytag, "_body");

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
  if(omode.r) return perror("2:dcmoutput_out -- recursive mode not available for CSV output"), 2;
  dcmoutput_csv(omode.outfile, meta, metatag, body, bodytag);
 }

 free(metatag);
 free(bodytag);

 return 0;
}
