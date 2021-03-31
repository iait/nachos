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
#include "string.h"

//-----------------------------------------------------------------------
// StartUserProgram
//      Función que invocará el nuevo thread.
//      Inicia la ejecución de un programa de usuario.
//
//-----------------------------------------------------------------------
void
StartUserProgram(void *arg)
{
    const char *args = (const char *) arg;
    DEBUG('u', "Iniciando la ejecución del programa de usuario <%s>\n", args);

    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    int i, j;

    // calcula la cantidad de argumentos y su longitud
    int argc = 1;
    for (i = 0; args[i] != '\0'; i++) {
        if (args[i] == ' ') {
            argc++;
        }
    }
    int len = i + 1;
    int align = divRoundUp(len, 4) * 4;

    // base del stack (límite del address space)
    int base = currentThread->space->getSize();

    // copia los argumentos en el fondo del stack
    int value;
    bool newArg = true;
    for (i = 0, j = 0; i < len; i++) {
        int addr = base - align + i;
        if (newArg) {
            machine->WriteMem(base - align - (argc - j) * 4, 4, addr);
            j++;
            newArg = false;
        }
        if (args[i] == ' ') {
            value = '\0';
            newArg = true;
        } else {
            value = args[i];
        }
        machine->WriteMem(addr, 1, value);
    }

    // inicializa el stack pointer y pasa en r4, r5 el argc, argv respectivamente
    int sp = base - align - argc * 4 - 16;
    machine->WriteRegister(StackReg, sp);
    machine->WriteRegister(4, argc);
    machine->WriteRegister(5, base - align - argc * 4);

    //machine->DumpMem(sp, base);

    machine->Run();
    ASSERT(false);
}

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
SpaceId
StartProcess(const char *program)
{
    int i;
    for (i = 0; program[i] != ' ' && program[i] != '\0'; i++);
    int len = i + 1;
    char filename[len];
    for (i = 0; i < len - 1; i++) {
        filename[i] = program[i];
    }
    filename[i] = '\0';

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

    thread->Fork(StartUserProgram, (void *) program);

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
