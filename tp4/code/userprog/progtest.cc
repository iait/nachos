// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "synchconsole.h"
#include "syscall.h"

//-----------------------------------------------------------------------
// StartUserProgram
//      Función que invocará el nuevo thread.
//      Inicia la ejecución de un programa de usuario.
//
//-----------------------------------------------------------------------
void
StartUserProgram(void *arg)
{
    char *name = (char *) arg;
    DEBUG('u', "Iniciando la ejecución del programa de usuario %s\n", name);

    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    machine->Run();
    ASSERT(false);
}

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
SpaceId
StartProcess(const char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        currentThread->Finish(-1);
        ASSERT(false);
        return -1;
    }
    space = new AddrSpace(executable);
    Thread *thread = new Thread(filename, true);
    thread->space = space;

    delete executable;

    thread->Fork(StartUserProgram, (void *) filename);

    return thread->getSpaceId();
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(void* arg) { readAvail->V(); }
static void WriteDone(void* arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------
void 
ConsoleTest (const char *in, const char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}

//----------------------------------------------------------------------
// SynchConsoleTest
//      Test the synch console by echoing characters typed at the input onto
//      the output. Stop when the user types a 'q'.
//----------------------------------------------------------------------
void
SynchConsoleTest(const char *in, const char *out)
{
    char ch;

    SynchConsole *sc = new SynchConsole(in, out);
    for (;;) {
        ch = sc->GetChar();
        sc->PutChar(ch);
        if (ch == 'q') {
            delete sc;
            return;
        }
    }
}
