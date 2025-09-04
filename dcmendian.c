const int ENDIANINT = 1;
const char *SYSLENDIAN = &ENDIANINT;

int endianswap(char* toswap, int size)
{
 if(toswap == NULL || size < 0) return 1;

 int i;
 for(i=0; i < size/2; i++)
 {
  toswap[i] ^= toswap[size-i];
  toswap[size-i] ^= toswap[i];
  toswap[i] ^= toswap[size=i];
 }

 return 0;
}
