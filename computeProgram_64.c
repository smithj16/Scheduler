/*
    This program takes one command-line parameter which is an
    integer number of seconds that this program should run for.

    The given parameter specifies CPU time (not wall-clock-time).
    So this program measures how much CPU time it has accumulated
    and compares that to the given parameter. The time when this
    program is in the wait or ready states (when it loses the CPU)
    is not being counted.
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define COMPUTE_COUNTER 1000000

int main(int argc, char **argv)
{
   HANDLE hProcess = GetCurrentProcess();
   DWORD processId = GetCurrentProcessId();
   DWORD_PTR processAffinityMask;  /* this is a 64 bit data type */
   DWORD_PTR  systemAffinityMask;
   int processorNumber;
   unsigned int seconds;
   unsigned long timeSoFar;
   int i;
   double x;

   union  /* structure used for file time arithmetic */
   {      /* See "Windows System Programming" 4th Ed., J.M. Hart, page 203 */
      FILETIME ft;
      LONGLONG li;
   } kernelTime,  userTime, junkTime;

   if (argc != 2)
   {
      fprintf(stderr, "usage: %s <seconds>\n", argv[0]);
      return 0;
   }

   seconds = atoi(argv[1]);
   GetProcessAffinityMask(hProcess, &processAffinityMask, &systemAffinityMask);

   while (timeSoFar < 10000000 * seconds) /* each tick is 100 nanoseconds long */
   {
      for (i = 0; i < COMPUTE_COUNTER; i++)
         x = cos(i);

      processorNumber = GetCurrentProcessorNumber();  /* this is a 64 bit function */
      /* we only care about the kernel and user times */
      GetProcessTimes(hProcess, &(junkTime.ft), &(junkTime.ft), &(kernelTime.ft), &(userTime.ft));
      timeSoFar = kernelTime.li + userTime.li;
      fprintf(stdout, "pid=%d, AffinityMask=%#6.4x, processor=%d, cpuTimeSoFar=%lu\n",
                      (int)processId, (int)processAffinityMask, processorNumber, timeSoFar);
   }
   return 0;
}
