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
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------
AddrSpace::AddrSpace(OpenFile *executable)
{
    unsigned int i, j, size, numPagesCode;

    exec = executable;
    exec->ReadAt((char *) &noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC)) {
        SwapHeader(&noffH);
    }
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;

    DEBUG('u', "Tamaño del address space: total=%d, code=%d, initData=%d, uninitData=%d\n",
                    size, noffH.code.size, noffH.initData.size, noffH.uninitData.size);

    numPagesCode = divRoundDown(noffH.code.size, PageSize); // número de páginas
                                                            // que solo tienen código

    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(((int) numPages) <= memMap->NumClear()); // check we're not trying to run anything too big

    DEBUG('u', "Initializing address space num pages %d, size %d\n", numPages, size);
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
        pageTable[i].readOnly = (i < numPagesCode);    // si la página solamente tiene código
                                                        // la puedo marcar como read-only
#ifdef USE_TLB
        pageTable[i].init = false;
#else
        bzero(machine->mainMemory + j * PageSize, PageSize);  // inicializa en cero la página
        // copia segmento de code e initData si corresponde
        if (noffH.code.size > 0) {
            CopyToAddrSpace(noffH.code, i);
        }
        if (noffH.initData.size > 0) {
            CopyToAddrSpace(noffH.initData, i);
        }
        pageTable[i].init = true;
#endif
    }
//    DumpPageTable();
//    memMap->Print();

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
   delete exec;
}

//----------------------------------------------------------------------
// AddrSpace::LoadPageInTlb
//      Carga una entrada nueva desde la tabla de paginación en la TLB.
//      Si la página no está inicializada en la memoria física lee del
//      ejecutable e inicializa la página.
//----------------------------------------------------------------------

void
AddrSpace::LoadPageInTlb(int vpn)
{
    if ((unsigned int) vpn >= numPages) {
        DEBUG('a', "virtual page # %d too large for page table size %d!\n",
                        vpn, numPages);
        machine->RaiseException(AddressErrorException, 0);
        return;
    }
    if (!pageTable[vpn].valid) {
        DEBUG('a', "virtual page # %d not valid!\n", vpn);
        machine->RaiseException(AddressErrorException, 0);
        return;
    }
    int index = machine->tlbIndex;
    machine->tlb[index] = pageTable[vpn];
    machine->tlbIndex = (index + 1) % TLBSize;

    // si la página no está inicializada
    if (!pageTable[vpn].init) {
        int physAddr = pageTable[vpn].physicalPage * PageSize;

        bzero(machine->mainMemory + physAddr, PageSize); // inicializa en cero la página

        // copia segmento de code e initData si corresponde
        if (noffH.code.size > 0) {
            CopyToAddrSpace(noffH.code, vpn);
        }
        if (noffH.initData.size > 0) {
            CopyToAddrSpace(noffH.initData, vpn);
        }

        pageTable[vpn].init = true;
    }
}

//----------------------------------------------------------------------
// AddrSpace::CopyToAddrSpace
//      Revisa si el segmento "seg" y la página virtual "virtPage" se intersecan.
//      Si es así, copia el contenido del ejecutable correspondiente.
//-----------------------------------------------------------------------
void
AddrSpace::CopyToAddrSpace(Segment seg, int virtPage) {

    int segFrom = seg.virtualAddr;
    int segTo = segFrom + seg.size;

    int virtAddrFrom = virtPage * PageSize;
    int virtAddrTo = virtAddrFrom + PageSize;

    DEBUG('u', "Segmento from: %d, to:%d; página %d from: %d, to: %d\n",
                    segFrom, segTo, virtPage, virtAddrFrom, virtAddrTo);

    if (segFrom < virtAddrTo && segTo > virtAddrFrom) {
        int from = segFrom < virtAddrFrom ? virtAddrFrom : segFrom;
        int to = segTo > virtAddrTo ? virtAddrTo : segTo;
        DEBUG('u', "Se graba from: %d, to: %d\n", from, to);

        int offset = from - virtAddrFrom;
        int numBytes = to - from;

        int physAddr = pageTable[virtPage].physicalPage * PageSize + offset;
        int posInFile = seg.inFileAddr + (segFrom < virtAddrFrom ? virtAddrFrom - segFrom : 0);

        int ret = exec->ReadAt(&(machine->mainMemory[physAddr]), numBytes, posInFile);
        ASSERT(ret == numBytes);
    }
}

//----------------------------------------------------------------------
// AddrSpace::WriteMem
//      Intenta grabar en memoria dos veces.
//      La primera falla puede darse porque falta la entrada en la tlb,
//      por lo que se vuelve a reintentar.
//      Si el segundo intento falla, aborta.
//----------------------------------------------------------------------

void
AddrSpace::WriteMem(int addr, int size, int value)
{
    ASSERT(interrupt->getStatus() != UserMode);
    if (!machine->WriteMem(addr, size, value)) {
#ifdef USE_TLB
        if (!machine->WriteMem(addr, size, value)) {
            ASSERT(false);
        }
#else
        ASSERT(false);
#endif
    }
}

//----------------------------------------------------------------------
// AddrSpace::ReadMem
//      Intenta leer de memoria dos veces.
//      La primera falla puede darse porque falta la entrada en la tlb,
//      por lo que se vuelve a reintentar.
//      Si el segundo intento falla, aborta.
//----------------------------------------------------------------------

void
AddrSpace::ReadMem(int addr, int size, int *value)
{
    ASSERT(interrupt->getStatus() != UserMode);
    if (!machine->ReadMem(addr, size, value)) {
#ifdef USE_TLB
        if (!machine->ReadMem(addr, size, value)) {
            ASSERT(false);
        }
#else
        ASSERT(false);
#endif
    }
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
#ifdef USE_TLB
    machine->InitializeTlb();
#else
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
}

void
AddrSpace::DumpPageTable()
{
    for (unsigned int i = 0; i < numPages; i++) {
        TranslationEntry entry = pageTable[i];
        printf("virtPage=%d, physPage=%d, valid=%s, readOnly=%s, use=%s, dirty=%s\n",
                        entry.virtualPage, entry.physicalPage,
                        entry.valid ? "true" : "false",
                        entry.readOnly ? "true" : "false",
                        entry.use ? "true" : "false",
                        entry.dirty ? "true" : "false");
    }
}
