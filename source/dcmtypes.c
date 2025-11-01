/*
 dcmtypes.c

 types that don't have any special functions associated directly with them
*/

#ifdef USESTDINT
#include <stdint.h>
#endif

#define DCMTYPES 1

/***
 use stdint if available
***/
typedef char byte1;
#ifdef _STDINT_H
 typedef uint16_t byte2;
 typedef uint32_t byte4;
 typedef uint64_t byte8;
 typedef   int8_t sbyte1;
 typedef  int16_t sbyte2;
 typedef  int32_t sbyte4;
 typedef  int64_t sbyte8;
#else
 typedef unsigned short byte2;
 typedef unsigned int   byte4;
 typedef unsigned long  byte8;
 typedef signed   char  sbyte1;
 typedef signed   short sbyte2;
 typedef signed   int   sbyte4;
 typedef signed   long  sbyte8;
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
 char* infname;
 unsigned int current;
 unsigned int last;
} outmode;

typedef enum
{
 l_close,
 l_open,
 l_write
} m_logmode;

typedef struct
{
 unsigned int  help;
 unsigned int  version;
 unsigned int  csv;
 char         *dir;
 char         *file;
 unsigned int  json;
 char         *log;
 char         *mode;
 unsigned int  number;
 char         *output;
 char         *prefix;
 unsigned int  recurse;
 char         *search;
 unsigned int  yaml;
} flagbreakout;

typedef struct
{
 byte2  *us;
 sbyte2 *ss;
 byte4  *ui;
 sbyte4 *si;
 byte8  *ul;
 sbyte8 *sl;
 float  *f;
 double *d;
} nums;
