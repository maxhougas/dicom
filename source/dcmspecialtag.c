/*
 Provides functions for checking special dicom elements
*/

#include"sqtags.c"
#ifdef _DCMTYPES
#include"dcmtypes.c"
#endif

/*
 From DICOM standard part 5 section 7.1.2
 these vrs imply a 2 byte length length with explicit vrs
*/
int dcmspecialtag_isshortvr(char *vr)
{
 const char *VRSHORTS[] = {"AE","AS","AT","CS","DA","DS","DT","FL","FD","IS","LO","LT","PN","SH","SL","SS","ST","TM","UI","UL","US"};
 const int NVRSHORT = (sizeof(VRSHORTS)/sizeof(char*));
 int i;
 for(i=0;i<NVRSHORT && /*strncmp(vr,VRSHORTS[i],2)*/*(byte2*)vr != *((byte2**)VRSHORTS)[i]; i++);
 return i<NVRSHORT;
}

/*
 From DICOM standard part 5 section 7.5
 these two tags end elements of undefined length (0xFFFFFFFF)
 {item delimitation group, item delimitation element, sequence delimitation group, sequence delimitation element}
*/
int dcmspecialtag_isdelimitation(int tag)
{
 const int NNDELIMITATION = 2;
 const int DELIMITATION[] = {0xFFFEE00D,0xFFFEE0DD};
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
int dcmspecialtag_isnovr(int tag)
{
 const int NNNOVRS = 3;
 const int NOVRS[] = {0xFFFEE000,0xFFFEE00D,0xFFFEE0DD};
 const int NNOVRS = (sizeof(NOVRS)/sizeof(int));
 int i;
 for(i=0; i < NNOVRS && tag != NOVRS[i]; i++);
 return i < NNOVRS;
}

int dcmspecialtag_issq(int tag)
{
 int NSQS = (sizeof(SQTAGS)/sizeof(int));
 int low = 0, high = NSQS-1, mid;
 do                                                                                                                       
 {
  mid = (high - low)/2;
  if(tag == SQTAGS[mid]) return 1;
  if(tag > SQTAGS[mid]) low = mid;
  else high = mid;
 } while(low < high);
 
 return 0;
}
