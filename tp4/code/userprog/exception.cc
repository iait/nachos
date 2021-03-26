// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synchconsole.h"

// External functions used by this file
SpaceId StartProcess(const char *file);

// Internal functions
char *ReadStringFromMem(int addr);
char *ReadStringFromMem(int addr, int numBytes);
void WriteStringToMem(int addr, int numBytes, char *buffer);
int ReadFromStdIn(int numBytes, char *buffer);
void WriteToStdOut(int numBytes, char *buffer);
void Dump();

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if (which == SyscallException) {
        switch (type) {
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Exit:
            {
                DEBUG('u', "Programa de usuario hace llamada a Exit.\n");
                int status = machine->ReadRegister(4);
                DEBUG('u', "Se termina hilo %s con estado %d\n", currentThread->getName(), status);
                currentThread->Finish(status);
                break;
            }
            case SC_Exec:
            {
                DEBUG('u', "Programa de usuario hace llamada a Exec.\n");
                int nameVirtAddr = machine->ReadRegister(4);
                char *name = ReadStringFromMem(nameVirtAddr);
                SpaceId spaceId = StartProcess(name);
                machine->WriteRegister(2, spaceId);
                break;
            }
            case SC_Join:
            {
                DEBUG('u', "Programa de usuario hace llamada a Join.\n");
                SpaceId spaceId = machine->ReadRegister(4);
                int exitStatus = scheduler->Join(spaceId);
                machine->WriteRegister(2, exitStatus);
                break;
            }
            case SC_Create:
            {
                DEBUG('u', "Programa de usuario hace llamada a Create.\n");
                int nameVirtAddr = machine->ReadRegister(4);
                char *name = ReadStringFromMem(nameVirtAddr);
                bool ret = fileSystem->Create(name, 0);
                if (!ret) {
                    printf("No se pudo crear el archivo %s\n", name);
                    free(name);
                    currentThread->Finish(-1);
                    ASSERT(false);
                }
                DEBUG('u', "Se crea el archivo %s.\n", name);
                free(name);
                break;
            }
            case SC_Open:
            {
                DEBUG('u', "Programa de usuario hace llamada a Open.\n");
                int nameVirtAddr = machine->ReadRegister(4);
                char *name = ReadStringFromMem(nameVirtAddr);
                OpenFile *file = fileSystem->Open(name);
                if (file == NULL) {
                    printf("No se pudo abrir el archivo %s\n", name);
                    free(name);
                    currentThread->Finish(-1);
                    ASSERT(false);
                }
                OpenFileId fileId = currentThread->AgregarDescriptor(file);
                machine->WriteRegister(2, fileId);
                DEBUG('u', "Se abre el archivo %s con descriptor %d\n", name, fileId);
                free(name);
                break;
            }
            case SC_Read:
            {
                DEBUG('u', "Programa de usuario hace llamada a Read.\n");
                int addr = machine->ReadRegister(4);
                int numBytes = machine->ReadRegister(5);
                OpenFileId fileId = machine->ReadRegister(6);
                char buffer[numBytes + 1];
                int read;
                if (fileId == 0) { // stdin
                    read = ReadFromStdIn(numBytes, buffer);
                } else if (fileId == 1) { // stdout
                    printf("Se intenta leer de la salida estándar.\n");
                    currentThread->Finish(-1);
                    ASSERT(false);
                } else {
                    OpenFile *file = currentThread->GetDescriptor(fileId);
                    if (file == NULL) {
                        printf("No se encuentra el archivo con descriptor %d\n", fileId);
                        currentThread->Finish(-1);
                        ASSERT(false);
                    }
                    read = file->Read(buffer, numBytes);
                }
                WriteStringToMem(addr, read, buffer);
                machine->WriteRegister(2, read);
                buffer[read] = '\0'; // para debug
                DEBUG('u', "Se lee <%s> del archivo con descriptor %d\n", buffer, fileId);
                //Dump();
                break;
            }
            case SC_Write:
            {
                DEBUG('u', "Programa de usuario hace llamada a Write.\n");
                int addr = machine->ReadRegister(4);
                int numBytes = machine->ReadRegister(5);
                OpenFileId fileId = machine->ReadRegister(6);
                char *content = ReadStringFromMem(addr, numBytes);
                if (fileId == 0) { // stdin
                    printf("Se intenta escribir en la entrada estándar.\n");
                    currentThread->Finish(-1);
                    ASSERT(false);
                } else if (fileId == 1) { // stdout
                    WriteToStdOut(numBytes, content);
                } else {
                    OpenFile *file = currentThread->GetDescriptor(fileId);
                    if (file == NULL) {
                        printf("No se encuentra el archivo con descriptor %d\n", fileId);
                        currentThread->Finish(-1);
                        ASSERT(false);
                    }
                    int ret = file->Write(content, numBytes);
                }
                DEBUG('u', "Se escribe <%s> en el archivo con descriptor %d\n", content, fileId);
                break;
            }
            case SC_Close:
            {
                DEBUG('u', "Programa de usuario hace llamada a Close.\n");
                OpenFileId fileId = machine->ReadRegister(4);
                ASSERT(fileId > 1);
                bool ret = currentThread->BorrarDescriptor(fileId);
                if (!ret) {
                    printf("No se encuentra el archivo con descriptor %d\n", fileId);
                    currentThread->Finish(-1);
                    ASSERT(false);
                }
                DEBUG('u', "Se cierra el archivo con descriptor %d\n", fileId);
                break;
            }
            default:
                printf("Llamada a sistema inesperada %d\n", type);
                ASSERT(false);
                break;
        }
        machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
    } else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(false);
    }
}

//-------------------------------------------------------------
// Dump
//
//      Para debug.
//-------------------------------------------------------------
void Dump()
{
        for (int i = 0; i < MemorySize; i++) {
                printf("%d: %c\n", i, machine->mainMemory[i]);
        }
}

//-------------------------------------------------------------
// ReadStringFromMem
//
//      Copia una cadena finalizada en '\0' desde la memoria principal.
//-------------------------------------------------------------
char *ReadStringFromMem(int addr)
{
        int n, i = 0;
        char str[1024];

        do {
            bool result = machine->ReadMem(addr++, 1, &n);
            ASSERT(result);
            str[i++] = (char) n;
        } while (n != '\0' && i < 1024);
        ASSERT(i < 1024);

        char *ret = (char *) malloc(sizeof(char) * i);
        strcpy(ret, str);

        return ret;
}

//-------------------------------------------------------------
// ReadStringFromMem
//
//      Copia una cadena desde la memoria principal hasta el número de bytes
//      indicado. Le agrega '\0' al final de la cadena solo para debug.
//-------------------------------------------------------------
char *ReadStringFromMem(int addr, int numBytes)
{
        int n, i = 0;
        char *ret = (char *) malloc(sizeof(char) * (numBytes + 1));

        do {
            bool result = machine->ReadMem(addr++, 1, &n);
            ASSERT(result);
            ret[i++] = (char) n;
        } while (i < numBytes);
        ret[i] = '\0';

        return ret;
}

//-------------------------------------------------------------
// WriteStringToMem
//
//      Copia una cadena hacia la memoria principal.
//-------------------------------------------------------------
void WriteStringToMem(int addr, int numBytes, char *buffer)
{
        for (int i = 0; i < numBytes; i++) {
            machine->WriteMem(addr++, 1, (int) buffer[i]);
        }
}

//-------------------------------------------------------------
// ReadFromStdIn
//
//      Lee desde la entrada estándar "numBytes" y los guarda en "buffer".
//      Espera a que haya un byte disponible.
//      Retorna el número de bytes leídos.
//-------------------------------------------------------------
int ReadFromStdIn(int numBytes, char *buffer)
{
        char ch;
        int i = 0;
        do {
            ch = synchConsole->GetChar();
            buffer[i] = ch;
            i++;
        } while (ch != EOF && i < numBytes);
        return i;
}

//-------------------------------------------------------------
// WriteToStdOut
//
//      Escribe a la salida estándar "numBytes" de "buffer".
//-------------------------------------------------------------
void WriteToStdOut(int numBytes, char *buffer)
{
        for (int i = 0; i < numBytes; i++) {
            synchConsole->PutChar(buffer[i]);
        }
}
