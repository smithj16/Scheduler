/* 
   Course: CS309
   Name: Jacob Smith
   Assignment: 3
 
*/
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

/* affinity mask of this processor (just one bit set) */
/* process currently running on this processor */
/* 1 when this processor is running a task, 0 otherwise */
typedef struct processor_data {
   int affinityMask;                
   PROCESS_INFORMATION processInfo; 
   int running;                     
} ProcessorData;


/* function prototypes */
void printError(char* functionName);
void SJF (int argc, char *argv[], int arr[]);  
void LJF (int argc, char *argv[], int arr[]); 



int main(int argc, char *argv[])
{
   int seconds[argc-2];
   int masks[8];
   int totalRuns = 0, next = 0, scheduleType;
   int processorCount = 0, temp;       /* the number of allocated processors */
   int handleCount = 0;
   ProcessorData *processorPool; /* an array of ProcessorData structures */
   HANDLE *processHandles;       /* an array of handles to processes */
   HANDLE thisProcess = GetCurrentProcess();
   DWORD mask, holder;
   STARTUPINFO start;
   ZeroMemory(&start, sizeof(start));
   start.cb = sizeof(start);
   DWORD processId = GetCurrentProcessId();

   if (argc < 3)
   {
      fprintf(stderr, "usage, %s  SCHEDULE_TYPE  SECONDS...\n", argv[0]);
      fprintf(stderr, "Where: SCHEDULE_TYPE = 0 means \"first come first serve\"\n");
      fprintf(stderr, "       SCHEDULE_TYPE = 1 means \"shortest job first\"\n");
      fprintf(stderr, "       SCHEDULE_TYPE = 2 means \"longest job first\"\n");
      return 0;
   }
   
  
   scheduleType = atoi(argv[1]); /* read scheduleType off the command-line */
   
   printf("Scheduling job duration times: "); 
   for(int i = 2; i<argc; i++){    /* read the job duration times off the command-line */
      seconds[i-2] = atoi(argv[i]);
      printf("%d  ", seconds[i-2]);
   }
   
  if(scheduleType == 1){
     SJF(argc, argv, seconds);
   }
   
  else if(scheduleType == 2){
     LJF(argc, argv, seconds);
   }
   
   GetProcessAffinityMask(thisProcess, &mask, &holder); /* get the processor affinity mask for this process */
   temp = mask;
   
   for(int i = 0; temp != 0; i++){ /* count the number of processors set in the affinity mask */
      if(temp %2){
         for(int k = 0; k<i; k++){
            masks[processorCount] = (temp % 2) * 2;
         }
         processorCount++;
      }
      temp /= 2;
   }
   
   for (int i = 0; i < processorCount; i++) 
   {
                if (processorPool[i].running) 
                {
                    handleCount++;
                }
   }
   
   processHandles = malloc(processorCount * sizeof(HANDLE));
   processorPool = malloc(processorCount * sizeof(ProcessorData)); /* create, and then initialize, the processor pool data structure */
   
   printf("\n processAffinityMask: %#6.4x  processorCount: %d  processId: %d \n", mask, processorCount, processId ); 
   char *currentDirectory = malloc(90);
   char *program = malloc(20);
   GetCurrentDirectory(90, currentDirectory);
   for(int i = 0; (i<processorCount) && (i< (int)(argc)-2); i++)
   {
 /* Get current directory for computeProgram_64 and the time in seconds */
      processorPool[i].affinityMask = masks[i];
      
      GetCurrentDirectory(90, currentDirectory);
      sprintf(program, "\\computeProgram_64 %d", seconds[i]);
      currentDirectory = strcat(currentDirectory, program);
      if( !CreateProcess(NULL, currentDirectory, NULL, NULL, FALSE,
                        NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED | CREATE_NEW_CONSOLE,
                        NULL, NULL, &start, &processorPool[i].processInfo) )
         {
            printError("CreateProcess");
         }
      else{
         SetProcessAffinityMask(processorPool[i].processInfo.hProcess, processorPool[i].affinityMask);
         processHandles[i] = processorPool[i].processInfo.hProcess;
         ResumeThread(processorPool[i].processInfo.hThread);
         totalRuns++;
         next++;
      }
      
   }
   
     
  /* Repeatedly wait for a process to finish and then,
      if there are more jobs to run, run a new job on
      the processor that just became free. */
  while (totalRuns > 0)
   {
     
      DWORD result;
      
      if (WAIT_FAILED == (result = WaitForMultipleObjects(handleCount, processHandles, FALSE, INFINITE)))
         printError("WaitForMultipleObjects");
      else{
         printf("Process %d has finished ", next-totalRuns);
         totalRuns--;
         processorPool[result].running = 0;
         SuspendThread(processorPool[result].processInfo.hThread);
         printf("handle: %d \n", processorPool[result].processInfo.hProcess);
         if(next<argc-2){
         
            /* Get current directory for computeProgram_64 and the time in seconds */
            char *currentDirectory = malloc(90);
            char *program = malloc(20);
            GetCurrentDirectory(90, currentDirectory);
            sprintf(program, "\\computeProgram_64 %d", seconds[next]);
            currentDirectory = strcat(currentDirectory, program);
            
            /* Create process at index i in processorPool */
            if( !CreateProcess(NULL, currentDirectory, NULL, NULL, FALSE,
                        NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED | CREATE_NEW_CONSOLE,
                        NULL, NULL, &start, &processorPool[result].processInfo) )
            {
            printError("CreateProcess");
            }
            SetProcessAffinityMask(processorPool[result].processInfo.hProcess, processorPool[result].affinityMask);
            processHandles[result] = processorPool[result].processInfo.hProcess;
            ResumeThread(processorPool[result].processInfo.hThread);
            next++;
            totalRuns++;
            processorPool[result].running = 1;
            printf("start process with handle: %d \n", processorPool[result].processInfo.hProcess);
         }
      }
      
   }
   
   for(int i =0; i<processorCount && i<argc-2;i++){
      CloseHandle(processorPool[i].processInfo.hProcess);
      CloseHandle(processorPool[i].processInfo.hThread);
   }
   return 0;
}

void SJF (int argc, char *argv[], int arr[]){

     bool flag = true; 
     if(argc >= 3 ){
        while(flag){
            flag = false;
            for(int i = 0; i < argc-3; i++){
               if(arr[i]>arr[i+1]){
                  arr[i] += arr[i+1];
                  arr[i+1] = arr[i] - arr[i+1];
                  arr[i] -= arr[i+1];
                  flag = true;
               }
            }
         } 
      }
      
      else{
          fprintf(stderr, "usage, %s  SCHEDULE_TYPE  SECONDS...\n", argv[0]);
         fprintf(stderr, "Where: SCHEDULE_TYPE = 0 means \"first come first serve\"\n");
         fprintf(stderr, "       SCHEDULE_TYPE = 1 means \"shortest job first\"\n");
         fprintf(stderr, "       SCHEDULE_TYPE = 2 means \"longest job first\"\n");
      }
 }
 
 
 void LJF (int argc, char *argv[], int arr[]){
      bool flag = true; 
      if(argc >= 3){
         while(flag){
            flag = false;
            for(int i = 0; i < argc-3; i++){
               if(arr[i] < arr[i+1]){
                  arr[i] += arr[i+1];
                  arr[i+1] = arr[i] - arr[i+1];
                  arr[i] -= arr[i+1];
                  flag = true;
               }
            }
         }
      }
      
      else{
          fprintf(stderr, "usage, %s  SCHEDULE_TYPE  SECONDS...\n", argv[0]);
         fprintf(stderr, "Where: SCHEDULE_TYPE = 0 means \"first come first serve\"\n");
         fprintf(stderr, "       SCHEDULE_TYPE = 1 means \"shortest job first\"\n");
         fprintf(stderr, "       SCHEDULE_TYPE = 2 means \"longest job first\"\n");
      }
   }
   
/****************************************************************
   The following function can be used to print out "meaningful"
   error messages. If you call a Windows function and it returns
   with an error condition, then call this function right away and
   pass it a string containing the name of the Windows function that
   failed. This function will print out a reasonable text message
   explaining the error.
*/
void printError(char* functionName)
{
   LPVOID lpMsgBuf;
   int error_no;
   error_no = GetLastError();
   FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         error_no,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* default language */
         (LPTSTR) &lpMsgBuf,
         0,
         NULL
   );
   /* Display the string. */
   fprintf(stderr, "\n%s failed on error %d: ", functionName, error_no);
   fprintf(stderr, (const char*)lpMsgBuf);
   /* Free the buffer. */
   LocalFree( lpMsgBuf );
}