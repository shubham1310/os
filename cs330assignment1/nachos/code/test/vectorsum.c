#include "syscall.h"
#define SIZE 10
int
main()
{
    int array[SIZE], i, sum=0;
        // system_PrintString("Vectorsum started\n"); //Added
        // system_Exec("../test/printtest");   //Added    
    for (i=0; i<SIZE; i++) array[i] = i;
    for (i=0; i<SIZE; i++) sum += array[i];
    system_PrintString("Total sum: ");
    system_PrintInt(sum);
    system_PrintChar('\n');
    system_PrintString("Executed instruction count: ");
    system_PrintInt(system_GetNumInstr());
    system_PrintChar('\n');
    system_Exit(0);
    return 0;
}
