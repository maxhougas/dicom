/*
 dcmtypes.c

 types that don't have any special functions associated directly with them
*/

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#define _DCMTYPES 1

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
 e_big,
 e_little
} m_endian;

typedef enum
{
 v_implicit,
 v_explicit
} m_vr;
