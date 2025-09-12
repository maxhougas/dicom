/*
 dcmspecialtag.c

 Provides functions for checking special dicom elements
*/

#include "sqtags.c"
#ifndef _DCMTYPES
#include "dcmtypes.c"
#endif

const int dcmspecialtag_ITEM = 0xFFFEE000;
const int dcmspecialtag_ITEMDELIM = 0xFFFEE00D;
const int dcmspecialtag_SEQUENCEDELIM = 0xFFFEE0DD;
const int dcmspecialtag_TSUID = 0x00020010;

/*
 From DICOM standard part 5 section 7.1.2
 these vrs imply a 2 byte length length with explicit vrs
*/
int dcmspecialtag_isshortvr(byte1 *vr)
{
 const char *VRSHORTS[] = {"AE","AS","AT","CS","DA","DS","DT","FL","FD","IS","LO","LT","PN","SH","SL","SS","ST","TM","UI","UL","US"};
 const int NVRSHORT = (sizeof(VRSHORTS)/sizeof(byte1*));
 int i;
 for(i=0;i<NVRSHORT && *(byte2*)vr != *((byte2**)VRSHORTS)[i]; i++);
 return i<NVRSHORT;
}

/*
 From DICOM standard part 5 section 7.5
 these two tags end elements of undefined length (0xFFFFFFFF)
 {item delimitation group, item delimitation element, sequence delimitation group, sequence delimitation element}
*/
int dcmspecialtag_isdelimitation(byte4 tag)
{
 const byte4 DELIMITATION[] = {0xFFFEE00D,0xFFFEE0DD};
 const int NDELIMITATION = (sizeof(DELIMITATION)/sizeof(int));
 int i;
 for(i=0; i < NDELIMITATION && tag != DELIMITATION[i]; i++);
 return i < NDELIMITATION;
}

/*
 From DICOM standard part 5 section 7.5
 these tags do not have vrs
 FFFE E000 = Item
 FFFE E00D = Item Delimitation Item
 FFFE E0DD = Sequence Delimitation Item
*/
int dcmspecialtag_isnovr(byte4 tag)
{
 const byte4 NOVRS[] = {0xFFFEE000,0xFFFEE00D,0xFFFEE0DD};
 const int NNOVRS = (sizeof(NOVRS)/sizeof(int));
 int i;
 for(i=0; i < NNOVRS && tag != NOVRS[i]; i++);
 return i < NNOVRS;
}

/*
 binary search for sq tag
 requires SQTAGS from sqtags.c
*/
int dcmspecialtag_issq(byte4 tag)
{
 int low = 0, high = NSQTAGS-1, mid;
 while((mid = (high + low)/2) != low)
 {
  mid = (high + low)/2;
  if(tag == SQTAGS[mid]) return 1;
  if(tag > SQTAGS[mid]) low = mid;
  else high = mid;
 } 

 return 0;
}

int dcmspecialtag_tsdecode(tsmode **mode, byte1* tsuid, int l)
{
 if(mode == NULL) {perror("1:dcmspecialtag_tsdecode"); return 1;}

 *mode = (tsmode*)malloc(sizeof(tsmode));
 if(*mode == NULL) {perror("2:dcmspecialtag_tsdecode"); return 2;}

 if(l == 18)
 {
  (*mode)->v = v_implicit;
  (*mode)->e = e_little;
  return 0;
 }

 byte1 important = tsuid[18];
 if(important == '1')
 {
  (*mode)->v = v_explicit;
  (*mode)->e = e_little;
 } 
 else if(important == '2')
 {
  (*mode)->v = v_explicit;
  (*mode)->e = e_big;
 }
 else /* dicom.nema.org says this is default :) */
 {
  (*mode)->v = v_implicit;
  (*mode)->e = e_little;
 }

 return 0;
}
