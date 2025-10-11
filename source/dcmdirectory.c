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

#define DCMDIRECTORY 1

#define DIRSL 0x400

int dcmdirectory_endir(char **dirs, char *dirname)
{
 if(dirname == NULL) return perror("1:endir -- dirname is null"), 1;

 struct dirent *de;
 DIR *dr = opendir(dirname);

 unsigned int sp = 0;
 unsigned int l;
 *dirs = malloc(DIRSL);
 unsigned int dirsmax = DIRSL;

 while((de = readdir(dr)) != NULL)
 {
  if(de->d_name[0] == '.') continue;

  l = strlen(de->d_name) + 1;
  if(sp + l >= dirsmax)
  {
   *dirs = realloc(*dirs, dirsmax + DIRSL);
   dirsmax += DIRSL;
   if(dirs == NULL) return perror("2:endir -- failed to expand dirs"), 2;
  }
  memcpy(&(*dirs)[sp], de->d_name, l);
  sp += l;
  (*dirs)[sp-1] = '\n';
 }

 (*dirs)[sp-1] = 0;

 return 0;
}

