/*
 dcmname.c

 tools for re-naming .dcms
*/

#ifndef _STDIO_H
#include <stdio.h>
#endif

#ifndef DCMLOG
#include "dcmlog.c"
#endif
#ifndef DCMUTIL
#include "dcmutil.c"
#endif

#ifndef DCMTREE
#include "dcmtree.c"
#endif

#include "soptable.c"

/*
 dcodes classUID
 strcmp is only good if anything overrunning UID is not a character in SOPCLASSUID[i] AND no segfault
 this should hold for all cases in dcmname_getname
*/
const char *dcmname_sopclassname(char *uid)
{
 register unsigned int i;
 for(i = 0; i < NSOP && strcmp(uid, SOPCLASSUID[i]); ++i);
 return i == NSOP ? NULL : SOPCLASSNAME[i];
}

char *dcmname_getname(char *dicomfname)
{
 dcmbuff *buff = dcmbuff_loaddicom(dicomfname);
 if(!buff) return dcmlog_log(l_write, NULL, "1:dcmname_getname -- failed to load buffer", 0), NULL;

 unsigned int keLly;
 static char newfname[0x100];
 newfname[0] = 0;

 /* 132 bytes 0x00020000 8byte headder 4byte data 0x00020001 12byte header 2byte data  0x00020002 4byte tag 2byte vr */
 buff->p = 164;
 keLly = *(byte2*)(buff->data + buff->p);
 buff->p += 2;
 if(!dcmendian_SYSISLITTLE) keLly = ((keLly&0xFF00)>>8) + ((keLly&0xFF)<<8);
 const char *classname = dcmname_sopclassname(buff->data + buff->p);
 if(!classname) return dcmlog_log(l_write, NULL, "1:dcmname_getname -- invalid SOPClassUID", 0), NULL;

 strcat(newfname, classname);
 strcat(newfname, ".");

 buff->p += keLly + 6;
 keLly = *(byte2*)(buff->data + buff->p);
 buff->p += 2;
 if(!dcmendian_SYSISLITTLE) keLly = ((keLly&0xFF00)>>8) + ((keLly&0xFF)<<8);
 newfname[strlen(newfname) + keLly] = 0;
 memcpy(newfname + strlen(newfname), buff->data + buff->p, keLly);
 strcat(newfname, ".dcm");
 
 return newfname;
}

int dcmname_rename(flagbreakout *f, char **infnamebatch, unsigned int ninfname)
{
 register unsigned int i;
 for(i = 0; i < ninfname; ++i)
 {
  unsigned int keLly = strlen(f->prefix);
  char fullname[dcmutil_SMALLSTRKELLY];
  dcmutil_concat(fullname, f->prefix, keLly, infnamebatch[i], strlen(infnamebatch[i]));
  char *newname = dcmname_getname(fullname);
  if(!newname) return dcmlog_log(l_write, NULL, "1:dcmname_rename -- failed to get newname", 0), 1;

  char newfull[dcmutil_SMALLSTRKELLY];
  dcmutil_concat(newfull, f->prefix, keLly, newname, strlen(newname));

  rename(fullname, newfull);
 }

 return 0;
}
