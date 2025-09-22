/*
 dcmtable.c

 functions for interacting with thetable
 thetable generated from DICOM standard part 6 sections 6, 7, 8, and 9
*/

#include "thetable.c"

#define DCMTABLE 1

#define dcmthetable_TAGS "tags"
#define dcmthetable_NAMES "names"
#define dcmthetable_KEYWORDS "keywords"
#define dcmthetable_VRS "vrs"
#define dcmthetable_VMS "vms"

/*
 binary search for thetable
*/
int dcmtable_tagsearch(byte4 tag)
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

/*
 linear search for thetable
*/
int dcmtable_wordsearch(m_table whichcol, char *word)
{
 char *col = THETABLE[whichcol == c_tags ? c_keywords : whichcol];

 int i;
 for(i = 0; i < NTHETABLE; i++)
  if(!strcmp(col, word)) return i;

 return -1;
}
