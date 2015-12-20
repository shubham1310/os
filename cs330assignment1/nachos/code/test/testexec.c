#include "syscall.h"

int
main()
{
    system_PrintString("Before calling Exec.\n");
    system_Exec("../test/forkjoin");
    system_PrintString("Returned from Exec.\n"); // Should never return
    return 0;
}
