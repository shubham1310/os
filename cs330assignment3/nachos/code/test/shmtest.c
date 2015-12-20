/* printtest.c
 *	Simple program to test whether printing from a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int
main()
{  
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
