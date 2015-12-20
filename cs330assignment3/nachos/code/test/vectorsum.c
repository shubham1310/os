/*#include "syscall.h"
#define SIZE 100
int
main()
{
    int array[SIZE], i, sum=0;

    for (i=0; i<SIZE; i++) array[i] = i;
    for (i=0; i<SIZE; i++) sum += array[i];
    system_PrintString("1Total sum: ");
    system_PrintInt(sum);
    system_PrintChar('\n');
    system_PrintString("Executed instruction count: ");
    system_PrintInt(system_GetNumInstr());
    system_PrintChar('\n');
    system_Exit(0);
    return 0;
}
*/

#include "syscall.h"

int
main()
{  
    system_PrintInt(10);
    system_PrintString("hello world\n");
    system_PrintInt(system_GetNumInstr());
    system_PrintString(" instructions.\n");
    int *array = (int*)system_ShmAllocate(2*sizeof(int));
    system_PrintInt(system_ShmAllocate(2*sizeof(int)));
     system_PrintString("hello world\n");

system_PrintInt(system_GetPA((unsigned)array));
array[0]=58;
array[1]=98;
    system_PrintString("Executed ");
system_PrintInt(array[0]);
system_PrintInt(array[1]);

return 0;
}