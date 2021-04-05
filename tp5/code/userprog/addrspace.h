// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "syscall.h"
#include "bitmap.h"
#include "noff.h"

#define UserStackSize		1024 	// increase this as necessary!

extern BitMap *memMap;

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable, SpaceId spaceId);
                                        // Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch

    int getSize() { return numPages * PageSize; }  // retorna el tamaño del address space

    void LoadPageInTlb(int virtPage);   // Carga la página en la TLB

    void WriteMem(int addr, int size, int value);   // lecturas y escrituras en memoria
    void ReadMem(int addr, int size, int *value);   // que hacen un reintento por si falta
                                                    // la página en la tlb
                                                    // solo para usar desde el sistema

  private:
    // agregado
    void CopyToAddrSpace(Segment seg, int virtAddr); // copia desde el ejecutable al address space
    void SaveToDisk(int vpn);                        // guarda la página al disco
    void RestoreFromDisk(int vpn);                   // restaura la página desde el disco

    void DumpPageTable(); // para debug
    void DumpExec();
    void DumpSwap();

    OpenFile *exec;                     // ejecutable del programa
    NoffHeader noffH;                   // cabecera del ejecutable

    OpenFile *swap;                     // archivo swap para paginación
    char *name;                         // nombre del archivo de paginación

    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space

    SpaceId spaceId;
};

#endif // ADDRSPACE_H
