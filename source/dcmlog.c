/*
 dcmlog.c

 logging functions
*/

#ifndef _STDIO_H
#include <stdio.h>
#endif
#ifndef _TIME_H
#include <time.h>
#endif

#define DCMLOG 1

/*
 nice formatting of time in GMT
 str must be at least 22 chars
*/
void dcmlog_formatgmt(char *str, time_t time)
{
 struct tm *stime = gmtime(&time);
 int month = stime->tm_mon + 1;
 int year = stime->tm_year + 1900;
            /*   34   67   90   23   56   8901*/
 sprintf(str,"%04d_%02d_%02d %02d:%02d:%02d Z%c", year, month, stime->tm_mday, stime->tm_hour, stime->tm_min, stime->tm_sec, 0);
}

/* 
 str must be at least 23 chars
 cputime cannot excede 2.7 hrs
 cputime precision cannot excede 10^-15 seconds
 hopefully this is future-proof enough
*/
void dcmlog_formatcputime(char *str, clock_t cputime)
{
 unsigned int pof10 = 0;
 unsigned int factor;
 for(factor = CLOCKS_PER_SEC; factor >= 10; factor /= 10)
  ++pof10;

 unsigned int cpusec = cputime / CLOCKS_PER_SEC;
 unsigned int subsec = cputime % CLOCKS_PER_SEC;
 /* if 200 cps, subsecs 100->50 indicating 1/2 second */
 subsec /= factor;
                     /*0123456789012345678*/
 char subsecstr[0x12] = "                 \0";
 sprintf(subsecstr,"%-16lu", subsec + CLOCKS_PER_SEC);
 subsecstr[strlen(subsecstr)] = ' ';
 sprintf(str,"%04u.%s", cpusec, &subsecstr[1]);
}

int dcmlog_log(m_logmode closeopenwrite, char *fname, char *message, clock_t ts)
{
 static FILE *log = NULL;

 if((closeopenwrite == l_write || closeopenwrite == l_close) && !log) return fprintf(stderr, "1:dcmlog_log -- logfile not open"), 1;
 if(closeopenwrite == l_open && log) return fprintf(log, " 2:dcmlog_log -- logfile already open\n"), 2;

 clock_t clocknow;
 time_t timenow;
 char clockstr[0x20];
 char timestr[0xF0];

 switch(closeopenwrite)
 {
 case l_open:
  log = strcmp("-", fname) ? fopen(fname, "a") : stderr;
  if(!log) return fprintf(stderr, "3:dcmlog_log -- failed to open log file %s\n", fname), 3;
  clocknow = clock();
  time(&timenow);
  dcmlog_formatcputime(clockstr, clocknow);
  dcmlog_formatgmt(timestr, timenow);
  fprintf(log, "%s : %s : Log file %s opened\n", timestr, clockstr, fname);
 break;
 case l_write:
  if(ts)
  {
   dcmlog_formatcputime(clockstr, ts);
   fprintf(log, " %s: %s\n", clockstr, message);
  }
  else
   fprintf(log, " %s\n", message);
 break;
 case l_close:
   clocknow = clock();
   time(&timenow);
   dcmlog_formatcputime(clockstr, clocknow);
   dcmlog_formatgmt(timestr, timenow);
  if(log != stderr)
  {
   fprintf(log, "%s : %s : Closing log file\n", timestr, clockstr);
   if(fclose(log)) fprintf(log, "4:dcmlog_log -- failed to close log file; continuing\n");
  }
  else
   fprintf(log, "%s : %s : 5:dcmlog_log -- refusing to close stderr; continuing\n", timestr, clockstr);
 break;
 }

 return 0;
}
