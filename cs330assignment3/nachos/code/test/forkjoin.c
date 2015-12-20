// #include "syscall.h"

// int
// main()
// {
//     int x;
//     int sleep_start, sleep_end;

//     system_PrintString("Parent PID: ");
//     system_PrintInt(system_GetPID());
//     system_PrintChar('\n');
//     x = system_Fork();
//     if (x == 0) {
//        system_PrintString("Child PID: ");
//        system_PrintInt(system_GetPID());
//        system_PrintChar('\n');
//        system_PrintString("Child's parent PID: ");
//        system_PrintInt(system_GetPPID());
//        system_PrintChar('\n');
//        sleep_start = system_GetTime();
//        system_Sleep(100);
//        sleep_end = system_GetTime();
//        system_PrintString("Child called sleep at time: ");
//        system_PrintInt(sleep_start);
//        system_PrintChar('\n');
//        system_PrintString("Child returned from sleep at time: ");
//        system_PrintInt(sleep_end);
//        system_PrintChar('\n');
//        system_PrintString("Child executed ");
//        system_PrintInt(system_GetNumInstr());
//        system_PrintString(" instructions.\n");
//     }
//     else {
//        system_PrintString("Parent after fork waiting for child: ");
//        system_PrintInt(x);
//        system_PrintChar('\n');
//        system_Join(x);
//        system_PrintString("Parent executed ");
//        system_PrintInt(system_GetNumInstr());
//        system_PrintString(" instructions.\n");
//        system_Sleep(10000);
//         x = system_Fork();
//       if (x == 0) {
//         system_PrintChar('\n'); 
//       }
//     }
//     return 0;
// }

#include "syscall.h"
#include "synchop.h"

#define SEM_KEY1 19		/* Key for semaphore protecting condition variables */
#define SEM_KEY2 20		/* Key for semaphore protecting stdout buffer */
#define COND_KEY1 19		/* Key for notFull condition variable */
#define COND_KEY2 20		/* Key for notEmpty condition variable */
#define SIZE 10			/* Size of queue */
#define NUM_ENQUEUER 4		/* Number of enqueuer threads */
#define NUM_DEQUEUER 5		/* Number of dequeuer threads */
#define DEQUEUE_EXIT_CODE 20	/* Exit code for dequeuer threads */
#define ENQUEUE_EXIT_CODE 10	/* Exit code for enqueuer threads */
#define NUM_ENQUEUE_OP 10	/* Number of operations per enqueuer thread */
#define NUM_DEQUEUE_OP (((NUM_ENQUEUE_OP*NUM_ENQUEUER)/NUM_DEQUEUER))	/* Number of operations per dequeuer thread */

int *array, semid, stdoutsemid, notFullid, notEmptyid;

int
Enqueue (int x, int id)
{
   int y;

   system_SemOp(semid, -1);
   while (array[SIZE+2] == SIZE) {
      system_SemOp(stdoutsemid, -1);
      system_PrintString("Enqueuer ");
      system_PrintInt(id);
      system_PrintString(": waiting on queue full.");
      system_PrintChar('\n');
      system_SemOp(stdoutsemid, 1);
      system_CondOp(notFullid, COND_OP_WAIT, semid);
   }
   array[array[SIZE+1]] = x;
   y = array[SIZE+1];
   array[SIZE+1] = (array[SIZE+1] + 1)%SIZE;
   array[SIZE+2]++;
   system_CondOp(notEmptyid, COND_OP_SIGNAL, semid);
   system_SemOp(semid, 1);
   return y;
}

int
Dequeue (int id, int *y)
{
   int x;

   system_SemOp(semid, -1);
   while (array[SIZE+2] == 0) {
      system_SemOp(stdoutsemid, -1);
      system_PrintString("Dequeuer ");
      system_PrintInt(id);
      system_PrintString(": waiting on queue empty.");
      system_PrintChar('\n');
      system_SemOp(stdoutsemid, 1);
      system_CondOp(notEmptyid, COND_OP_WAIT, semid);
   }
   x = array[array[SIZE]];
   (*y) = array[SIZE];
   array[SIZE] = (array[SIZE] + 1)%SIZE;
   array[SIZE+2]--;
   system_CondOp(notFullid, COND_OP_SIGNAL, semid);
   system_SemOp(semid, 1);
   return x;
}

int
main()
{
    array = (int*)system_ShmAllocate(sizeof(int)*(SIZE+3));  // queue[SIZE], head, tail, count
    int x, i, j, seminit = 1, y;
    int pid[NUM_DEQUEUER+NUM_ENQUEUER];
    system_PrintInt(array);

// system_PrintString("df\n");
// return 0;
    for (i=0; i<SIZE; i++) array[i] = -1;
    array[SIZE] = 0;
    array[SIZE+1] = 0;
    array[SIZE+2] = 0;
    system_PrintString("HERE");
    // return 0;

   semid = system_SemGet(SEM_KEY1);
    system_PrintString("Sem_id done\n");
    system_PrintInt(semid);
    system_SemCtl(semid, SYNCH_SET, &seminit);
    stdoutsemid = system_SemGet(SEM_KEY2);
    system_PrintInt(stdoutsemid);
    system_SemCtl(stdoutsemid, SYNCH_SET, &seminit);
    //system_PrintString("Done with second semctl\n");
    notFullid = system_CondGet(COND_KEY1);
    system_PrintString("CondGet works!\n");
    system_PrintInt(notFullid);
    notEmptyid = system_CondGet(COND_KEY2);
    system_PrintString("\nSecond CondGet works!\n");
    system_PrintInt(notEmptyid);

    for (i=0; i<NUM_DEQUEUER; i++) {
      //system_PrintString("\nEntered for loop\n");
       x = system_Fork();
       // system_PrintString("Fork Completed successfully!\n");
       if (x == 0) {
          for (j=0; j<NUM_DEQUEUE_OP; j++) {
            // x = Dequeue (i, &y);
             system_SemOp(stdoutsemid, -1);
             //system_PrintString("Returned from semop\n");
             system_PrintString("Dequeuer ");
             system_PrintInt(system_GetPID());
             system_PrintString(": Got ");
             system_PrintInt(x);
             system_PrintString(" from slot ");
             system_PrintInt(j);
             system_PrintChar('\n');
             system_SemOp(stdoutsemid, 1);
          }
          // break;
          // system_PrintInt(j);               //added
          // system_PrintString("\n");       //added
         system_Exit(DEQUEUE_EXIT_CODE);
       }
       pid[i] = x;
    }
    
    for (i=0; i<NUM_ENQUEUER; i++) {
       x = system_Fork();
       system_PrintString("x = ");
       system_PrintInt(x);
       if (x == 0) {
          x = i*NUM_ENQUEUE_OP;
          for (j=0; j<NUM_ENQUEUE_OP; j++) {
             y = Enqueue (x+j, i);
             system_SemOp(stdoutsemid, -1);
             system_PrintString("Enqueuer ");
             system_PrintInt(i);
             system_PrintString(": Inserted ");
             system_PrintInt(x+j);
             system_PrintString(" in slot ");
             system_PrintInt(y);
             system_PrintChar('\n');
             system_SemOp(stdoutsemid, 1);
          }
          system_Exit(ENQUEUE_EXIT_CODE);
       }
       pid[i+NUM_DEQUEUER] = x;
    }

    for (i=0; i<NUM_DEQUEUER+NUM_ENQUEUER; i++) {
       x = system_Join(pid[i]);
       system_SemOp(stdoutsemid, -1);
       system_PrintString("Parent joined with ");
       system_PrintInt(pid[i]);
       system_PrintString(" having exit code ");
       system_PrintInt(x);
       system_PrintChar('\n');
       system_SemOp(stdoutsemid, 1);
    }
    system_SemCtl(semid, SYNCH_REMOVE, 0);
    system_SemCtl(stdoutsemid, SYNCH_REMOVE, 0);
    system_CondRemove(notFullid);
    system_CondRemove(notEmptyid);

    return 0;
}