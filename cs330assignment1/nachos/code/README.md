											CS330  Assignment 1
										 		NACHOS
										 	   Group - 39
										 	Aayush Ojha -13009
										 	Amit Kumar - 13094
										 	Shubham Jain-13683

1. syscall_GetReg
File Changed : exception.cc
In this system call we have read the value of passed argument using machine->ReadRegister(4) and then using this value as the register number and similarly we got the register value. Now we used machine->WriteRegister() and wrote this value in the return register(register number 2).

2. syscall_GetPA
Files Changed: exception.cc
We find the number of pages the given address occupies by dividing the address by the PageSize i.e. 128. We now check if this value is lesser than the number of entries in page table by comparing it with machine->pageTableSize. Secondly, we check if the machine's pageTable at this value is valid or not. Thirdly, we check if the physical page number of this value is not larger than the number of physical pages(NumPhysPages defined in machine.h). If any of these conditions are violated then we return -1. Otherwise, we find the physical address using the formula, physAddr=pageFrame*PageSize+offset

3.syscall_GetPID
Files Changed: exception.cc ; thread.h; thread.cc
To get the pid we defined a new global variable pidcounter with initial value 0. Each time a thread is created we assign this valu as the pid of the thread and increment the pidcounter. To access the pid we defined getpid() method( public, since pid is a private variable).

4.syscall_GetPPID
Files Changed: exception.cc ; thread.h; thread.cc
Now each time a thread is created we assign ppid of newly made thread to be the pid of calling thread. To access the ppid we defined getppid() method( public, since ppid is a private variable).

5.syscall_Time
Files Changed: exception.cc ; 
To get the Total ticks at present we returned the value of totalTicks present in Statistics class using stats object(defined in system.cc) of that class.

6.syscall_Sleep
Files Changed: exception.cc ; thread.cc; thread.h;
We read the number of ticks for which the current process is to be put to sleep. If this value is zero, we simply call YieldCPU. Otherwise, we calculate the time(from start) at which this process is to be woken(put to ready queue). We store this value in the whenToWake variable(private, NachOSThread class) using setwhentowake() function.
Now, we add this thread in the Sleep list(list created for all the sleeping threads) in a sorted order(according to deadline) and set its status to blocked. We then created a timer which continuously raises interrupt after every tick. We then call currentThread->PutThreadToSleep which then puts the current calling thread to sleep. We defined a TimerInterruptHandler() function in exception.cc which handles the every interrupt raised by timer. In this function we first check if there is a process scheduled to wake up at present tick(using FindNextToWake()). For every such process we remove this process from sleep list(done in FindNextToWake) and add it to readyList and set its status to READY. While inserting or removing a process from a list we turn off the inturrupt and turn it on afterwards.


7.syscall_Yield
Files Changed: exception.cc;
We called the YieldCPU function in thread.cc with the current thread which put the calling thread to sleep and run the next to run thread. After returning from Yield we have simply advanced the program counter.

8.syscall_Fork
Files Changed: exception.cc; thread.cc; thread.h; addrspace.cc; addrspace.h;
We first advance the program counter as the child need to run from next instruction. We then save the register values of the parent registers using currentThread->SaveUserState(). We then create a new thread for the child with the name "ForkChild". Now we copy the setChild() with the currentThread i.e. parent. In the setChild() function, firstly  we copy the  parent stored register in the child register and  set the return value of child  register to 0(userRegisters[2]=0). We then create a new address space for the child. We defined a new constructor for AddrSpace class. This constructor initialises numPages to 0 and creat a Translation entry. We then call a function AddrSpace::ThreadCreateAddrSpace(addrspace.cc) and pass the current thread as argument. This function copy the context of parent to the child. We have set a offset spacemarker for the physical address thus it gives a unique physical address for each thread. Then we allocate a stack to the thread by calling ThreadFork function and after creating stack it pushes child thread to the readyToRun queue. After that we pushes the child into children list used in join and exit system call. Finally we return the pid of the child to the parent.

9.system_Exec
File Changed: exception.cc; 
We first read the file name passed as a argument using machine->ReadMem(file, 1, &temp).
Once we have the executable name we pass this to StartProcessExec(fName). StartProcessExec() is just the copy StartProcess() function defined in progtest.cc. This function creates a new address space for the exectuable passed. In the AddrSpace::AddrSpace(OpenFile *executable) constructor we have changed the way physical address is assigned by using add by using a spacemarker to prevent any owerwriting. After addrespace is assigned to the executable it will just initialises its registers and restores its state and simply push it to run thus now the new executable will run.

10.system_Join
File Changed: exception.cc; thread.cc; thread.h;scheduler.h; scheduler.cc;
We will search for whether the child passed is a child of current thread using SearchChildren(x) defined in thread.cc. This function search for the child in the children list. If it is not a child it simply returns -1. If the function return NULL, then we will first search whether the child has already exited using SearchExited(x) defined in scheduler.cc. If will search through a exited list created to store all the threads which is already exited along its exit code. If the child is already exited we just return the exit code of the child. If the child has not been exited then we call Scheduler::RunToWait () function, it changes the status of parent thread to BLOCKED and append it waitingList. waitingList is the list created to store all the threads which are waiting for its child to be waited on to complete. Waking the parent after the child is exited is handled in syscall_Exit().

11.syscall_Exit
File Changed: exception.cc; thread.h; thread.cc; scheduler.h; scheduler.cc;system.h;system.cc;
We define a new variable NumOfRunningThreads in system.h which store number of threads which are in any one of the following state- running(readyList), sleeping(sleepList), waiting(waitList) and I/O(queue). Hence, first we check if there is only one thread and that too is called for exited, then we just cleanup().If any thread remains in any of the four list mentioned above it does not cleanup() and just exit that thread. Before exiting the thread we append its pid along with its exit code in the exitedList to be used for syscall_Join. Now before finishing the thread we wake up its parent.This is done using WakeWaitingThread() function deifined in scheduler.cc.This function checks if there is any thread waiting for current thread to finish. If yes, then we remove it from waiting list and push it to readyList. After that we finish the thread by calling FinishThread() function.

12.syscall_GetNumInstr
Files Changed: exception.cc; thread.cc; thread.h ; mipssim.cc;
To get the number of instructions we created a new variable numInstr for each thread in NachOSClass. Each time machine::OneInstruction() is called, we increment(using incrementInstructions() function in thread.cc) the number of instructions for the thread which call the OneInstruction() function. We return the number of instructions for calling thread by calling currentThread->getInstructions().