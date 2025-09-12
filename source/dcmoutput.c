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
 int i,j;
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
  for(j = 0; j < el->length; j++)
  {
   fprintf(outfile, "0x%02X", j);
   if(j < el->length -1)
    fprintf(outfile,", ");
   else
    fprintf(outfile," ]\n"); 
  }
 }

 fprintf(outfile, "body: \n");
 for(i = 0; i < meta->p; i++)
 {
  el = body->els[i];
 fprintf(outfile, "- tag: 0x%08X\n", el->tag);
 fprintf(outfile, "  vr: %c%c\n", el->vr[0],el->vr[1]);
 fprintf(outfile, "  length: 0x%08X\n", el->length);
 fprintf(outfile, "  value: [ ");
  for(j = 0; j < el->length; j++)
  {
   fprintf(outfile, "0x%02X", j);
   if(j < el->length -1)
    fprintf(outfile,", ");
   else
    fprintf(outfile," ]\n"); 
  }
 }
}

int dcmoutput_json(FILE *outfile, dcmelarr *meta, dcmelarr *body)
{
 int i,j;
 dcmel *el;

 fprintf(outfile, "{\n");

 fprintf(outfile, " \"meta\": [\n");
 for(i = 0; i < meta->p; i++)
 {
  el = meta->els[i];
 fprintf(outfile, "  {\n");
 fprintf(outfile, "   \"tag\": %d,\n", el->tag);
 fprintf(outfile, "   \"vr\": \"%c%c\",\n", el->vr[0], el->vr[1]);
 fprintf(outfile, "   \"length\": %d,\n", el->length);
 fprintf(outfile, "   \"value\": [");
  for(j = 0; j < el->length; j++)
  {
   fprintf(outfile, "%d", el->data[j]);
   if(j < el->length - 1)
    fprintf(outfile, ",");
   else
    fprintf(outfile,"]\n");
  }
 fprintf(outfile, "  }");
  if(i < meta->p - 1)
   fprintf(outfile, ",\n");
  else
   fprintf(outfile, "\n");
 }
 fprintf(outfile, " ],\n");

 fprintf(outfile, " \"body\": [\n");
 for(i = 0; i < body->p; i++)
 {
  el = body->els[i];
 fprintf(outfile, "  {\n");
 fprintf(outfile, "   \"tag\": %d,\n", el->tag);
 fprintf(outfile, "   \"vr\": \"%c%c\",\n", el->vr[0], el->vr[1]);
 fprintf(outfile, "   \"length\": %d,\n", el->length);
 fprintf(outfile, "   \"value\": ]");
  for(j = 0; j < el->length; j++)
  {
   fprintf(outfile, "%d", el->data[j]);
   if(j < el->length - 1)
    fprintf(outfile, ",");
   else
    fprintf(outfile,"]\n");
  }
 fprintf(outfile, "  }");
  if(i < body->p - 1)
   fprintf(outfile, ",\n");
  else
   fprintf(outfile, "\n");
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
