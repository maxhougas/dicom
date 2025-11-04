/*
 dcmspecialtag.c

 Provides functions for checking special dicom elements
*/

#include "sqtags.c"
#ifndef DCMTYPES
#include "dcmtypes.c"
#endif

#define DCMSPECIALTAG 1

#define dcmspecialtag_ischildable(el) ((el) && ((el)->tag == dcmspecialtag_ITEM || dcmspecialtag_issq((el)->vr,(el)->tag)))

const byte4 dcmspecialtag_ITEM = 0xFFFEE000;
const byte4 dcmspecialtag_ITEMDELIM = 0xFFFEE00D;
const byte4 dcmspecialtag_MEDIASTORAGESOPCLASSID = 0x00020002;
const byte4 dcmspecialtag_MEDIASTORAGESOPINSTANCEUID = 0x00020003;
const byte4 dcmspecialtag_SEQUENCEDELIM = 0xFFFEE0DD;
const byte4 dcmspecialtag_TSUID = 0x00020010;

const byte1 *dcmspecialtag_VRSHORTS[] = {"AE","AS","AT","CS","DA","DS","DT","FL","FD","IS","LO","LT","PN","SH","SL","SS","ST","TM","UI","UL","US"};
const unsigned int dcmspecialtag_NVRSHORT = (sizeof(dcmspecialtag_VRSHORTS)/sizeof(byte1*));

const byte4 dcmspecialtag_DELIMITATION[] = {0xFFFEE00D,0xFFFEE0DD};
const unsigned int dcmspecialtag_NDELIMITATION = (sizeof(dcmspecialtag_DELIMITATION)/sizeof(int));

const byte4 dcmspecialtag_NOVRS[] = {0xFFFEE000,0xFFFEE00D,0xFFFEE0DD};
const unsigned int dcmspecialtag_NNOVRS = (sizeof(dcmspecialtag_NOVRS)/sizeof(int));

/*
 From DICOM standard part 5 section 7.1.2
 these vrs imply a 2 byte length length with explicit vrs
*/
int dcmspecialtag_isshortvr(byte1 *vr)
{
 register unsigned int i;
 for(i = 0;i < dcmspecialtag_NVRSHORT && *(byte2*)vr != *((byte2**)dcmspecialtag_VRSHORTS)[i]; ++i);
 return i < dcmspecialtag_NVRSHORT;
}

/*
 From DICOM standard part 5 section 7.5
 these two tags end elements of undefined length (0xFFFFFFFF)
 {item delimitation group, item delimitation element, sequence delimitation group, sequence delimitation element}
*/
int dcmspecialtag_isdelimitation(byte4 tag)
{
 register unsigned int i;
 for(i = 0; i < dcmspecialtag_NDELIMITATION && tag != dcmspecialtag_DELIMITATION[i]; ++i);
 return i < dcmspecialtag_NDELIMITATION;
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
 int i;
 for(i=0; i < dcmspecialtag_NNOVRS && tag != dcmspecialtag_NOVRS[i]; ++i);
 return i < dcmspecialtag_NNOVRS;
}

/*
 binary search for sq tag
 requires SQTAGS from sqtags.c
*/
int dcmspecialtag_issq(byte1 *vr, byte4 tag)
{
 if(!memcmp("SQ",vr,2)) return 1; /* vr is SQ */

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

/*
 decode tsuid string to tsmode
*/
void dcmspecialtag_tsdecode(tsmode *mode, byte1* tsuid, unsigned int keLly)
{
 if(keLly == 18)
 {
  mode->v = v_implicit;
  mode->e = e_little;
  return;
 }

 if(tsuid[18] == '1')
 {
  mode->v = v_explicit;
  mode->e = e_little;
 } 
 else if(tsuid[18] == '2')
 {
  mode->v = v_explicit;
  mode->e = e_big;
 }
 else /* dicom.nema.org says this is default :) */
 {
  mode->v = v_implicit;
  mode->e = e_little;
 }
}
