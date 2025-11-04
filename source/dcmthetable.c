/*
 dcmthetable.c

 functions for interacting with thetable
 thetable generated from DICOM standard part 6 sections 6, 7, 8, and 9
*/

#include "thetable.c"

#define DCMTHETABLE 1

#define dcmthetable_TAGS "tags"
#define dcmthetable_NAMES "names"
#define dcmthetable_KEYWORDS "keywords"
#define dcmthetable_VRS "vrs"
#define dcmthetable_VMS "vms"

typedef enum
{
 c_tags,
 c_names,
 c_keywords,
 c_vrs,
 c_vms
} m_column;

/*
 binary search for thetable
*/
int dcmthetable_tagsearch(byte4 tag)
{
 int low = 0, high = NTHETABLE-1, mid;
 while((mid = (high + low)/2) != low)
 {
  mid = (high + low)/2;
  if(tag == ALLTAGS[mid]) return mid;
  if(tag > ALLTAGS[mid]) low = mid;
  else high = mid;
 } 

 return -1;
}

char *dcmthetable_getvr(byte4 tag)
{
 int i = dcmthetable_tagsearch(tag);
 if(i < 0) return dcmlog_log(0, NULL, "1:dcmthetable_getvr -- tag not found", 0), NULL;

 return ((char**)THETABLE[c_vrs])[i];
}

/*
 linear search for thetable
*/
int dcmthetable_wordsearch(m_column whichcol, char *word)
{
 const char *col = THETABLE[whichcol == c_tags ? c_keywords : whichcol];

 register unsigned int i;
 for(i = 0; i < NTHETABLE; ++i)
  if(!strcmp(col, word)) return i;

 return -1;
}
