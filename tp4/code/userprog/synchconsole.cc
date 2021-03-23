// synchconsole.cc
//      Rutinas para el acceso síncrono al dispositivo de consola.
//

#include "synchconsole.h"

static void SynchReadAvail(void *arg) {
        SynchConsole *synchConsole = (SynchConsole *) arg;
        synchConsole->ReadAvail();
}
static void SynchWriteDone(void *arg) {
        SynchConsole *synchConsole = (SynchConsole *) arg;
        synchConsole->WriteDone();
}

//----------------------------------------------------------------------
// SynchConsole::SynchConsole
//
//      Inicializa la consola síncrona.
//
//      "readFile" -- UNIX file simulating the keyboard (NULL -> use stdin)
//      "writeFile" -- UNIX file simulating the display (NULL -> use stdout)
//----------------------------------------------------------------------
SynchConsole::SynchConsole(const char *readFile, const char *writeFile)
{
        console = new Console(readFile, writeFile, SynchReadAvail, SynchWriteDone, this);
        read = new Lock("read");
        write = new Lock("write");
        readAvail = new Semaphore("readAvail", 0);
        writeDone = new Semaphore("writeDone", 0);
}

//----------------------------------------------------------------------
// SynchConsole::~SynchConsole
//
//      Limpieza.
//----------------------------------------------------------------------
SynchConsole::~SynchConsole()
{
        delete console;
        delete read;
        delete write;
        delete readAvail;
        delete writeDone;
}

//----------------------------------------------------------------------
// SynchConsole::PutChar
//
//      Escribe el caracter en consola y retorna recién cuando se haya
//      completado la operación.
//----------------------------------------------------------------------
void
SynchConsole::PutChar(char ch)
{
        write->Acquire(); // solo se permite una escritura a la vez
        console->PutChar(ch);
        writeDone->P();   // bloquea hasta que el caracter haya sido escrito
        write->Release();
}

//----------------------------------------------------------------------
// SynchConsole::GetChar
//
//      Obtiene un caracter desde la consola.
//      Si no hay disponible, espera a que haya
//----------------------------------------------------------------------
char
SynchConsole::GetChar()
{
        read->Acquire(); // solo se permite una lectura a la vez
        readAvail->P();  // espero a que haya un caracter disponible
        char ch = console->GetChar();
        read->Release();
        return ch;
}

//----------------------------------------------------------------------
// SynchConsole::ReadAvail
//
//      Callback llamado por el manejador de interrupciones cuando hay
//      un caracter disponible para lectura.
//----------------------------------------------------------------------
void
SynchConsole::ReadAvail()
{
        readAvail->V();
}

//----------------------------------------------------------------------
// SynchConsole::WriteDone
//
//      Callback llamado por el manejador de interrupciones cuando se
//      completó la escritura.
//----------------------------------------------------------------------
void
SynchConsole::WriteDone()
{
        writeDone->V();
}

