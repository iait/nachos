// coremap.h
//      Estructuras de datos para definir coremap.
//
//      Mapa que indica a qué página virtual de qué proceso corresponde una
//      página física.
//

#ifndef COREMAP_H
#define COREMAP_H

#include "synch.h"
#include "addrspace.h"

class CoreEntry {
  public:
    int physicalPage;    // número de página física
    AddrSpace *space;    // espacio de direcciones al que corresponde
                         // NULL indica que no mapea ninguna página virtual
    int virtualPage;     // número de pagina virtual dentro del espacio de direcciones
};

#endif //COREMAP_H
