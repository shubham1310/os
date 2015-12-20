// system.cc 
//	Nachos initialization and cleanup routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.

NachOSThread *currentThread;			// the thread we are running now
NachOSThread *threadToBeDestroyed;  		// the thread that just finished
Scheduler *scheduler;			// the ready list
Interrupt *interrupt;			// interrupt status
Statistics *stats;			// performance metrics
Timer *timer;				// the hardware timer device,
					// for invoking context switches
/*
SchedType=1:- Non Preemptive Scheduling
SchedType=2:- Non Preemptive Scheduling with Shortest CPU Burst first
SchedType=3:- Round Robin
Schedtype=4:- Unix Scheduling based on priority
*/
int SchedType=1; //initial value will not matter
unsigned long long CompletionTime[MAX_THREAD_COUNT];
long long DiffCPUBurst=0;        // Store sum of absolute(S(n)-t(n))
unsigned numPagesAllocated;     // number of physical frames allocated
unsigned LastTime;              // Stores LastTime when a thread was scheduled to be used in preemptive scheduling
unsigned TimeQuanta=100;         // Time Quanta to be used in Preemptive scheduling         
NachOSThread *threadArray[MAX_THREAD_COUNT];  // Array of thread pointers
unsigned thread_index;                  // Index into this array (also used to assign unique pid)
bool initializedConsoleSemaphores;
bool exitThreadArray[MAX_THREAD_COUNT];  //Marks exited threads
bool UpdateReq=true;                     // Stores whether CPUsage is to be updated or not 
int CPUsage[MAX_THREAD_COUNT];           // Stores CPUsage of thread with PID
int CPUBurst[MAX_THREAD_COUNT];          // Stores CPUBurst of thread with PID
int Priority[MAX_THREAD_COUNT];          // Stores Priority of thread with PID
int waitInitial[MAX_THREAD_COUNT];       // Timestamp: when thread was put in Ready Queue
int S[MAX_THREAD_COUNT];                 // Stores S value for threads 
int basePriority[MAX_THREAD_COUNT];      // Stores basePriority= BASE_PRIORITY+priority specified
int sumBurst=0;                          // Stores sum of total cpu bursts for pid=1 only 
int coBurst=0;                           // Stores number of total cpu bursts for pid=1 only 
long long totCPUBurst=0;                       // Stores sum of total cpu bursts
int waitTime =0;                         // Stores sum of total time spend by Threads in Ready Queue
int maxBurst=0;
int minBurst=999999;

TimeSortedWaitQueue *sleepQueueHead;    // Needed to implement SC_Sleep

#ifdef FILESYS_NEEDED
FileSystem  *fileSystem;
#endif

#ifdef FILESYS
SynchDisk   *synchDisk;
#endif

#ifdef USER_PROGRAM	// requires either FILESYS or FILESYS_STUB
Machine *machine;	// user program memory and registers
#endif

#ifdef NETWORK
PostOffice *postOffice;
#endif

// External definition, to allow us to take a pointer to this function
extern void Cleanup();



// Return Priority of thread
int GetPriority (NachOSThread* t) {return Priority[t->GetPID()];}
// Return S of thread
int GetS(NachOSThread* t) {return S[t->GetPID()];}
// Set Priority of thread with that pid
void SetPriority(int p,int pid){basePriority[pid]=p+BASE_PRIORITY;Priority[pid]=basePriority[pid];}
// Update Priority of all threads
void UpdatePriority()
{
    if(CPUBurst[currentThread->GetPID()]==0) return ;
    for(int i =0;i<thread_index;i++)
    {
        int pid=i;
        // Update CPUsage as done in UNIX scheduling
        CPUsage[pid]=CPUsage[pid]+CPUBurst[pid];
        CPUBurst[pid]=0;
        CPUsage[pid]=CPUsage[pid]/2;
        // Update priority as done in UNIX scheduling
        Priority[pid]=basePriority[pid]+CPUsage[pid]/2;
    }   
    UpdateReq=true;
}
//----------------------------------------------------------------------
// TimerInterruptHandler
// 	Interrupt handler for the timer device.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling YieldCPU() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called YieldCPU at the point it is 
//	was interrupted.
//
//	"dummy" is because every interrupt handler takes one argument,
//		whether it needs it or not.
//----------------------------------------------------------------------
static void
TimerInterruptHandler(int dummy)
{
    TimeSortedWaitQueue *ptr;
    if (interrupt->getStatus() != IdleMode) {
        // Check the head of the sleep queue
        while ((sleepQueueHead != NULL) && (sleepQueueHead->GetWhen() <= (unsigned)stats->totalTicks)) {
           sleepQueueHead->GetThread()->Schedule();
           ptr = sleepQueueHead;
           sleepQueueHead = sleepQueueHead->GetNext();
           delete ptr;
        }
        //printf("[%d] Timer interrupt.\n", stats->totalTicks);
        // check if scheduling policy allows YieldOnReturn
        if(SchedType==1 || SchedType == 2) return;
        else if(SchedType==3 || SchedType==4)
        {
            int x=stats->totalTicks-LastTime;
            // If scheduling is preemptive check if current Quanta expires
            // if expired call YieldOnReturn to schedule next thread
            if(x>=TimeQuanta)
            interrupt->YieldOnReturn();
        }
        else
            printf("Not Valid Scheduling Type\n");//for debugging only
    }
}

//----------------------------------------------------------------------
// Initialize
// 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.  
// 
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
    int argCount, i;
    char* debugArgs = "";
    bool randomYield = FALSE;

    initializedConsoleSemaphores = false;
    numPagesAllocated = 0;

    for (i=0; i<MAX_THREAD_COUNT; i++) { threadArray[i] = NULL; exitThreadArray[i] = false; }
    thread_index = 0;

    sleepQueueHead = NULL;

#ifdef USER_PROGRAM
    bool debugUserProg = FALSE;	// single step user program
#endif
#ifdef FILESYS_NEEDED
    bool format = FALSE;	// format disk
#endif
#ifdef NETWORK
    double rely = 1;		// network reliability
    int netname = 0;		// UNIX socket name
#endif
    
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
	if (!strcmp(*argv, "-d")) {
	    if (argc == 1)
		debugArgs = "+";	// turn on all debug flags
	    else {
	    	debugArgs = *(argv + 1);
	    	argCount = 2;
	    }
	} else if (!strcmp(*argv, "-rs")) {
	    ASSERT(argc > 1);
	    RandomInit(atoi(*(argv + 1)));	// initialize pseudo-random
						// number generator
	    randomYield = TRUE;
	    argCount = 2;
	}
#ifdef USER_PROGRAM
	if (!strcmp(*argv, "-s"))
	    debugUserProg = TRUE;
#endif
#ifdef FILESYS_NEEDED
	if (!strcmp(*argv, "-f"))
	    format = TRUE;
#endif
#ifdef NETWORK
	if (!strcmp(*argv, "-l")) {
	    ASSERT(argc > 1);
	    rely = atof(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-m")) {
	    ASSERT(argc > 1);
	    netname = atoi(*(argv + 1));
	    argCount = 2;
	}
#endif
    }

    DebugInit(debugArgs);			// initialize DEBUG messages
    stats = new Statistics();			// collect statistics
    interrupt = new Interrupt;			// start up interrupt handling
    scheduler = new Scheduler();		// initialize the ready queue
    //if (randomYield)				// start the timer (if needed)
	timer = new Timer(TimerInterruptHandler, 0, randomYield);

    threadToBeDestroyed = NULL;

    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new NachOSThread("main");		
    currentThread->setStatus(RUNNING);

    interrupt->Enable();
    CallOnUserAbort(Cleanup);			// if user hits ctl-C
    
#ifdef USER_PROGRAM
    machine = new Machine(debugUserProg);	// this must come first
#endif

#ifdef FILESYS
    synchDisk = new SynchDisk("DISK");
#endif

#ifdef FILESYS_NEEDED
    fileSystem = new FileSystem(format);
#endif

#ifdef NETWORK
    postOffice = new PostOffice(netname, rely, 10);
#endif
}

//----------------------------------------------------------------------
// Cleanup
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------
void
Cleanup()
{
    printf("\nCleaning up...\n");
#ifdef NETWORK
    delete postOffice;
#endif
    
#ifdef USER_PROGRAM
    delete machine;
#endif

#ifdef FILESYS_NEEDED
    delete fileSystem;
#endif

#ifdef FILESYS
    delete synchDisk;
#endif
    
    delete timer;
    delete scheduler;
    delete interrupt;
    
    Exit(0);
}

