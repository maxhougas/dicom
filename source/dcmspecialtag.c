/*
 Provides functions for checking special dicom elements
*/

/*
 From DICOM standard part 5 section 7.1.2
 these vrs imply a 2 byte length length with explicit vrs
*/
int dcmspecialtag_isshortvr(char* vr)
{
 const char* VRSHORTS[] = {"AE","AS","AT","CS","DA","DS","DT","FL","FD","IS","LO","LT","PN","SH","SL","SS","ST","TM","UI","UL","US"};
 const int NVRSHORT = 21;
 int i;
 for(i=0;i<NVRSHORT && strncmp(VRSHORTS[i],vr,2);i++);
 return i<NVRSHORT;
}

/*
 From DICOM standard part 5 section 7.5
 these two tags end elements of undefined length (0xFFFFFFFF)
 {item delimitation group, item delimitation element, sequence delimitation group, sequence delimitation element}
*/
int dcmspecialtag_isdelimitation(int tag)
{
 const int NDELIMITATION = 2;
 const int DELIMITATION[] = {0xFFFEE00D,0xFFFEE0DD};
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
 const int NNOVRS = 3;
 const int NOVRS[] = {0xFFFEE000,0xFFFEE00D,0xFFFEE0DD};
 int i;
 for(i=0; i < NNOVRS && tag != NOVRS[i]; i++);
 return i < NNOVRS;
}

int dcmspecialtag_getsqsfromfile(char *fname)
{
 static int *sqsl = NULL;
 static int *sqsh = NULL;
 static int *sqs = NULL;

 if(sqs == NULL)
 {
  sqsl = malloc(sizeof(int)*NSQS);
  sqsh = malloc(sizeof(int)*NSQS);
  char buff[LSQS+1];
  char *end;
  FILE *fsqs = fopen(PSQS,"r");
  

  unsigned int i;

  fgets(buff,LSQS,fsqs);
  for(i=0; i<NSQS && !feof(fsqs); i++);
  {
   sqsl[i] = (unsigned int)strtoul(buff, &end, 16);
   sqsh[i] = (unsigned int)strtoul(&buff[9], &end, 16);
   if(sqsl[i] == 0 || sqsh[i] == 0) return -1;
   fgets(buff,LSQS,fsqs);
  }

  fclose(fsqs);
 }
}

int dcmspecialtag_issq(int tag)
{

 int low = 0, high = NSQS-1, mid;
 do                                                                                                                       
 {
  mid = (high - low)/2;
  if(tag >= sqsl[mid] && tag <= sqsh[mid]) return 1;
  if(tag > sqsl[mid]) low = mid;
  else high = mid;
 } while(low <= high);
 
 return 0;
}
