/*
 dcmendian.c

 tools for dealing with endianness

 unpointering dcmendian_SYSISLITTLE gives the system endianness
 dcmendian_swap swaps the endianness of an arbitrary type
 dcmendian_4flip swaps the endianness of a 4-byte integer
 dcmendian_handletag deals with the tag being 2 2-byte integers
*/
 
#ifndef DCMTYPES
#include "dcmtypes.c"
#endif

#define DCMENDIAN 1

#define dcmendian_4flip(a) ((((a)&0xFF000000)>>24) + (((a)&0xFF0000)>>8) + (((a)&0xFF00)<<8) + (((a)&0xFF)<<24))

const int dcmendian_ENDIANINT = 1;
const char *dcmendian_SYSISLITTLE = (char*)&dcmendian_ENDIANINT;

int dcmendian_swap(byte1* toswap, int size)
{
 if(toswap == NULL || size < 0) {perror("1:dcmendian_swap"); return 1;}

 int i;
 for(i=0; i < size/2; i++)
 {
  toswap[i] ^= toswap[size-i];
  toswap[size-i] ^= toswap[i];
  toswap[i] ^= toswap[size=i];
 }

 return 0;
}

/*
 tag is represented in file as gggg,eeee
 i.e. 2 2-byte numbers. 1 4-byte number is moar better
*/
int dcmendian_handletag(byte4* tag, m_endian file_endianness)
{
 if(*dcmendian_SYSISLITTLE && file_endianness == e_little)
  *tag = ((*tag&0xFFFF0000)>>16) + ((*tag&0xFFFF)<<16);
 else if(*dcmendian_SYSISLITTLE && file_endianness == e_big)
  *tag = ((*tag&0xFF000000)>>24) + ((*tag&0xFF0000)>>8) + ((*tag&0xFF00)<<8) + ((*tag&0xFF)<<24);
 else if(!*dcmendian_SYSISLITTLE && file_endianness == e_little)
  *tag = ((*tag&0xFF000000)>>8) + ((*tag&0xFF0000)<<8) + ((*tag&0xFF00)>>8) + ((*tag&0xFF)<<8);

 return 0;
}
