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
#include "synchlist.h"
#include "synch.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

class Scheduler {
  public:
    Scheduler();			// Initialize list of ready threads 
    ~Scheduler();			// De-allocate ready list

    void ReadyToRun(Thread* thread);	// Thread can be dispatched.
    Thread* FindNextToRun();		// Dequeue first thread on the ready 
					// list, if any, and return thread.
    void Run(Thread* nextThread);	// Cause nextThread to start running
    void Print();			// Print contents of ready list

    // agregado para multicolas de prioridad
    void Promote(Thread *thread);       // Promueve a la prioridad del currentThread

    // agregado para multiprogramación
    void Finish(SpaceId spaceId, int exitStatus); // Agrega el hilo a la lista de terminados con su estado
    int Join(SpaceId spaceId);                    // Espera a que el hilo termine y devuelve su estado

  private:
    List<Thread*> *readyList;  		// queue of threads that are ready to run,
					// but not running
    List<Thread*> *readyListP[11];	// Colas de prioridad

    // agregado para multiprogramación
    SynchList<int> *statusList;         // lista de los estados de salida de los hilos
};

#endif // SCHEDULER_H
