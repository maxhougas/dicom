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
}

void dcmoutput_yamlrecurse(dcmel *el, FILE* outfile, unsigned int depth)
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
   dcmoutput_yamlrecurse(el->children[i], outfile, depth + 1);
 }
 else
 {
  fprintf(outfile, "[ ");
  if(el->length > 0)
  {
   for(i = 0; i < el->length - 1; i++)
    fprintf(outfile, "%02X, ", el->data[i]);
   fprintf(outfile, "%02X ", el->data[i]);
  }
  fprintf(outfile, "]\n");
 }
}

int dcmoutput_json(FILE *outfile, dcmelarr *meta, dcmelarr *body)
{
 unsigned int i;
 dcmel *el;

 void printel()
 {
  fprintf(outfile, "  {\n");
  fprintf(outfile, "   \"tag\": %d,\n", el->tag);
  fprintf(outfile, "   \"vr\": \"%c%c\",\n", el->vr[0], el->vr[1]);
  fprintf(outfile, "   \"length\": %d,\n", el->length);
  fprintf(outfile, "   \"value\": [");
  if(el->length > 0)
  {
   unsigned int j;
   for(j = 0; j < el->length - 1; j++)
    fprintf(outfile, "%d, ", el->data[j]);
   fprintf(outfile, "%d", el->data[j]);
  }
  fprintf(outfile,"]\n");
  fprintf(outfile, "  }");
 }

 fprintf(outfile, "{\n");

 fprintf(outfile, " \"meta\": [\n");
 if(meta->p > 0)
 {
  for(i = 0; i < meta->p - 1; i++)
  {
   el = meta->els[i];
   printel();
   fprintf(outfile, ",\n");
  }
  el = meta->els[i];
  printel();
  fprintf(outfile, "\n");
 }
 fprintf(outfile, " ],\n");

 fprintf(outfile, " \"body\": [\n");
 if(body->p > 0)
 {
  for(i = 0; i < body->p - 1; i++)
  {
   el = body->els[i];
   printel();
   fprintf(outfile,",\n");
  }
  el = body->els[i];
  printel();
  fprintf(outfile,"\n");
 }
 fprintf(outfile, " ]\n");
 
 fprintf(outfile, "}");
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
 
  for(j = 0; j < el->length; j++)
   fprintf(outfile, "0x%02X ", el->data[j]);
  fprintf(outfile, "\n");
 }

 fprintf(outfile, "***BODYSTART***\n");
 
 for(i = 0; i < body->p; i++)
 {
  el = body->els[i];
  fprintf(outfile, "0x%08X,%c%c,%d,",el->tag, el->vr[0], el->vr[1], el->length);
 
  for(j = 0; j < el->length; j++)
   fprintf(outfile, "0x%02X ", el->data[j]);
  fprintf(outfile, "\n");
 }
}

int dcmoutput_out(char *outfname, m_format format, dcmelarr *meta, dcmelarr *body)
{
 int i,j;
 dcmel *el;
 FILE *outfile = strcmp("-",outfname) ? fopen(outfname, "w") : stdout;
 if(outfile == NULL) {perror("1:dcmoutput_out"); return 1;}

 switch(format)
 {
 case f_yaml:
  dcmoutput_yaml(outfile, meta, body);
 break;
 case f_json:
  dcmoutput_json(outfile, meta, body);
 break;
 case f_csv:
  dcmoutput_csv(outfile, meta, body);
 break;
 default:
  perror("2:dcmoutput_out"); return 2;
 } 

 int err = outfile == stdin ? 0 : fclose(outfile);
 if(err) {perror("3:dcmoutput_out"); return 3;}

 return 0;
}
