// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

BitMap *memMap = new BitMap(NumPhysPages);

void DumpPageTable();

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::CopyToAddrSpace
//      Copia "bytesToCopy" bytes desde la posición "posInFile" del archivo
//      "file" al address space iniciando desde la dirección virtual "virtAddr".
//-----------------------------------------------------------------------
void
AddrSpace::CopyToAddrSpace(OpenFile *file, int posInFile, int bytesToCopy, int virtAddr)
{
    int virtPage = virtAddr / PageSize;
    int offset = virtAddr % PageSize;
    while (bytesToCopy > 0) {
        int physAddr = pageTable[virtPage].physicalPage * PageSize + offset;
        int numBytes = bytesToCopy < (PageSize - offset) ? bytesToCopy : (PageSize - offset);
        int ret = file->ReadAt(&(machine->mainMemory[physAddr]), numBytes, posInFile);
        ASSERT(ret == numBytes);
        bytesToCopy -= numBytes;
        posInFile += numBytes;
        virtPage++;
        offset = 0;
    }
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------
AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, j, size, numPagesCode;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPagesCode = divRoundDown(noffH.code.size, PageSize); // número de páginas
                                                            // que solo tienen código
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(((int) numPages) <= memMap->NumClear());	// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space num pages %d, size %d\n", numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
        j = memMap->Find();  // busca una página física libre
        ASSERT(j >= 0);
	pageTable[i].virtualPage = i;
	pageTable[i].physicalPage = j;
	pageTable[i].valid = true;
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = (i <= numPagesCode);    // si la página solamente tiene código
                                                        // la puedo marcar como read-only

	bzero(machine->mainMemory + j * PageSize, PageSize);  // inicializa en cero la página
    }
//    DumpPageTable();
//    memMap->Print();

// then, copy the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('u', "Initializing code segment\n");
        CopyToAddrSpace(executable, noffH.code.inFileAddr,
                        noffH.code.size, noffH.code.virtualAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('u', "Initializing data segment\n");
        CopyToAddrSpace(executable, noffH.initData.inFileAddr,
                        noffH.initData.size, noffH.initData.virtualAddr);
    }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   // marca como libres las páginas del proceso que se destruye.
   for (unsigned int i = 0; i < numPages; i++) {
      memMap->Clear(pageTable[i].physicalPage);
   }
   delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

void
AddrSpace::DumpPageTable()
{
    for (unsigned int i = 0; i < numPages; i++) {
        printf("%d: %d\n", i, pageTable[i].physicalPage);
    }
}
