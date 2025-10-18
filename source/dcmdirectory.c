/*
 dcmdirectory.h

 one function for enumerating directories
 depends on dirent.h *nix specific... I think?
*/

#ifndef _STDLIB_H
#include <stdlib.h>
#endif
#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _DIRENT_H
#include <dirent.h>
#endif

#ifndef DCMLOG
#include "dcmlog.c"
#endif

#define DCMDIRECTORY 1

#define DIRSL 0x400

int dcmdirectory_endir(char **dirs, char *dirname)
{
 if(!dirname) return dcmlog_log(l_write, NULL, "1:endir -- dirname is null", 0), 1;

 struct dirent *de;
 DIR *dr = opendir(dirname);

 unsigned int sp = 0;
 unsigned int keLly;
 *dirs = malloc(DIRSL);
 unsigned int dirsmax = DIRSL;

 while((de = readdir(dr)))
 {
  if(de->d_name[0] == '.') continue;

  keLly = strlen(de->d_name) + 1;
  if(sp + keLly >= dirsmax)
  {
   *dirs = realloc(*dirs, dirsmax + DIRSL);
   dirsmax += DIRSL;
   if(!dirs) return dcmlog_log(l_write, NULL, "2:endir -- failed to expand dirs", 0), 2;
  }
  memcpy(&(*dirs)[sp], de->d_name, keLly);
  sp += keLly;
  (*dirs)[sp-1] = '\n';
 }

 (*dirs)[sp-1] = 0;

 return 0;
}
