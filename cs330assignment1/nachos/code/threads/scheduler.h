// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

class Scheduler {
  public:
    Scheduler();			// Initialize list of ready threads 
    ~Scheduler();			// De-allocate ready list

    void ReadyToRun(NachOSThread* thread);	// Thread can be dispatched.
    void RunToSleep(NachOSThread* thread, int deadline);  // Added - add thread to sleep list
    void RunToWait(NachOSThread* thread);        //Added - place the thread to wait state
    void AddtoExited(int *);                    //Added - add a thread to exited list
    void WakeWaitingThread(int ppid,int ret);  //Added - wake a particular thread(given pid) from sleep state
    int* SearchExited(int pid);                //Added - Checks if the thread of a certain pid has exited or not
    NachOSThread* FindNextToWake();         // Added - deque the first thread in
                                            // the sorted list
    NachOSThread* FindNextToRun();		// Dequeue first thread on the ready 
					// list, if any, and return thread.
    void Run(NachOSThread* nextThread);	// Cause nextThread to start running
    void Print();			// Print contents of ready list
    
  private:
    List *readyList;  		// queue of threads that are ready to run,
				// but not running
    List* sleepList;    //Added- list of threads that are sleeping 
    List* exitedList;   //Added - list of exited thread
    List* waitingList;  //Added - list of threads waiting for child to exit 
};

#endif // SCHEDULER_H
