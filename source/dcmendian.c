/*
 dcmendian.c

 tools for dealing with endianness

 unpointering dcmendian_SYSISLITTLE gives the system endianness
 dcmendian_swap swaps the endianness of an arbitrary type
 dcmendian_4flip swaps the endianness of a 4-byte integer
 dcmendian_handletag deals with the tag being 2 2-byte integers
*/

/*
 std library headers
*/
#ifndef _LIMITS_H
#include <limits.h>
#endif
 
/*
 internals
*/
#ifndef DCMTYPES
#include "dcmtypes.c"
#endif

#define DCMENDIAN 1

/*
 this monstrosity should be bit width insensitive
*/
#define BBITS  (UCHAR_MAX+1)
#define B0(a)  ((a)&UCHAR_MAX)
#define B1(a)  ((a)&(UCHAR_MAX*BBITS))
#define B2(a)  ((a)&(UCHAR_MAX*BBITS*BBITS))
#define B3(a)  ((a)&(UCHAR_MAX*BBITS*BBITS*BBITS))
#define B4(a)  ((a)&(UCHAR_MAX*BBITS*BBITS*BBITS*BBITS))
#define B5(a)  ((a)&(UCHAR_MAX*BBITS*BBITS*BBITS*BBITS*BBITS))
#define B6(a)  ((a)&(UCHAR_MAX*BBITS*BBITS*BBITS*BBITS*BBITS*BBITS))
#define B7(a)  ((a)&(UCHAR_MAX*BBITS*BBITS*BBITS*BBITS*BBITS*BBITS*BBITS))
#define R1B(a) ((a)/BBITS)
#define R3B(a) ((a)/BBITS/BBITS/BBITS)
#define R5B(a) ((a)/BBITS/BBITS/BBITS/BBITS/BBITS)
#define R7B(a) ((a)/BBITS/BBITS/BBITS/BBITS/BBITS/BBITS)
#define L1B(a) ((a)*BBITS)
#define L3B(a) ((a)*BBITS*BBITS*BBITS)
#define L5B(a) ((a)*BBITS*BBITS*BBITS*BBITS*BBITS)
#define L7B(a) ((a)*BBITS*BBITS*BBITS*BBITS*BBITS*BBITS)
#define dcmendian_2flip(a) (R1B(B1(a)) + L1B(B0(a))
#define dcmendian_4flip(a) (R3B(B3(a)) + R1B(B2(a)) + L1B(B1(a)) + L3B(B0(a)))
#define dcmendian_8flip(a) (R7B(B7(a)) + R5B(B6(a)) + R3B(B5(a)) + R1B(B4(a)) + L1B(B3(a)) + L3B(B2(a)) + L5B(B1(a)) + L7B(B0(a))

const int dcmendian_ENDIANINT = 1;
const char *dcmendian_SYSISLITTLE = (char*)&dcmendian_ENDIANINT;

/*
 endian swap arbitrary data types
 MUTATES
*/
void dcmendian_swap(byte1* toswap, unsigned int size)
{
 int i;
 for(i=0; i < size/2; ++i)
 {
  toswap[i] ^= toswap[size - i];
  toswap[size - i] ^= toswap[i];
  toswap[i] ^= toswap[size - i];
 }
}

/*
 tag is represented in file as gggg,eeee
 i.e. 2 2-byte numbers. 1 4-byte number is moar better
 MUTATES
*/
void dcmendian_handletag(byte4* tag, m_endian file_endianness)
{
 if(*dcmendian_SYSISLITTLE && file_endianness == e_little)
  *tag = ((*tag&0xFFFF0000)>>16) + ((*tag&0xFFFF)<<16);
 else if(*dcmendian_SYSISLITTLE && file_endianness == e_big)
  *tag = ((*tag&0xFF000000)>>24) + ((*tag&0xFF0000)>>8) + ((*tag&0xFF00)<<8) + ((*tag&0xFF)<<24);
 else if(!*dcmendian_SYSISLITTLE && file_endianness == e_little)
  *tag = ((*tag&0xFF000000)>>8) + ((*tag&0xFF0000)<<8) + ((*tag&0xFF00)>>8) + ((*tag&0xFF)<<8);
}
