/*
 dcmtypes.c

 types that don't have any special functions associated directly with them
*/

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#define DCMTYPES 1

/***
 use stdint if available
***/
#ifdef _STDINT_H
 typedef uint8_t byte1;
 typedef uint16_t byte2;
 typedef uint32_t byte4;
#else
 typedef unsigned char byte1;
 typedef unsigned short byte2;
 typedef unsigned int byte4;
#endif

typedef enum
{
 v_implicit,
 v_explicit
} m_vr;

typedef enum
{
 e_big,
 e_little
} m_endian;

typedef enum
{
 f_csv,
 f_json,
 f_yaml
} m_format;

typedef enum
{
 r_norecurse,
 r_recurse
} m_recurse;

typedef enum
{
 c_tags,
 c_names,
 c_keywords,
 c_vrs,
 c_vms
} m_column;

typedef struct
{
 m_vr v;
 m_endian e;
} tsmode;

typedef struct
{
 m_format f;
 m_recurse r;
 FILE *outfile;
 char *tag;
 unsigned int current;
 unsigned int last;
} outmode;
