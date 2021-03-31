// synchconsole.h
//      Estructura para soportar acceso síncrono al dispositivo de consola.
//

#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "console.h"
#include "synch.h"

class SynchConsole {
  public:
    SynchConsole(const char *readFile, const char *writeFile);

    ~SynchConsole();

    void PutChar(char ch); // Escribe el caracter en consola y retorna recién
                           // cuando se haya completado la operación.

    char GetChar();        // Obtiene un caracter desde la consola.
                           // si no hay disponible, espera a que haya

    // rutinas internas
    void ReadAvail();      // callback llamado por el manejador de interrupciones
                           // cuando hay un caracter disponible para lectura
    void WriteDone();      // callback llamado por el manejador de interrupciones
                           // cuando se completó la escritura
private:
    Console *console;
    Lock *read, *write;
    Semaphore *readAvail, *writeDone;
};

#endif // SYNCHCONSOLE_H
