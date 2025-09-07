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

