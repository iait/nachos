Práctica 5
----------

En la clase AddrSpace:
    Se agrega método LoadPageInTlb para cargar una nueva entrada en la tabla de paginación 
    descartando la más vieja (FIFO).

    Dentro del método RestoreState se inicializa la TLB poniendo como no válidas todas sus entradas.

    Se agregan métodos ReadMem y WriteMem que reintentan la lectura/escritura si se está utilizando 
    TLB. Estos métodos solo se usarán desde el sistema en modo kernel. En modo usuario, el reintento
    se produce por no aumentar el program counter.

En exception.cc se agrega el manejo de la excepción PageFaultException que hace que se cargue la 
entrada de la página correspondiente en la TLB.

Se arregla el programa de usuario sort.c que no estaba ordenando bien el array. El resultado que se 
espera luego de ordenar el array en forma ascendente es 1 y no de 0 como indicaba el comentario, ya 
que 1 es el menor de los valores que se guarda en el array. Por otro lado, el resultado que se 
espera luego de ejecutar el programa matmult.c que realiza una multiplicación de matrices es 7220.
Se agrega la opción de debug "e" con la que se imprime un mensaje con el estado de salida de los 
programas que realizan la llamada a sistema Exit. Por ejemplo, al ejecutar los programas "sort" y 
"matmult" se obtienen las siguientes salidas:

$ ../vm/nachos -x sort -d e
Se termina hilo <sort> con estado 1

$ ../vm/nachos -x matmult -d e
Se termina hilo <matmult> con estado 7220

Se agrega en stats.h variables para obtener el número de aciertos y fallos de pagina.

Al ejecutar el programa "sort" se obtuvieron los siguientes resultados:
Con TLBSize=32, Page hit: 44.056.119, page miss: 5.115, porcentaje de aciertos: 99,9883912%
Con TLBSize=64, Page hit: 44.051.505, page miss:    39, porcentaje de aciertos: 99,9999115%
Cantidad de páginas totales de su espacio de direcciones: 46 pages

Al ejecutar el programa "matmult" se obtuvieron los siguientes resultados:
Con TLBSize=32, Page hit: 709.417, page miss: 106, porcentaje de aciertos: 99,9850604%
Con TLBSize=64, Page hit: 709.365, page miss:  46, porcentaje de aciertos: 99,9935157%
Cantidad de páginas totales de su espacio de direcciones: 53 pages

Vemos que claramente al aumentar el tamaño de la TLB se producen más aciertos de páginas. De hecho, 
las tablas de paginación de ambos programas entran completamente en la TLB cuando su tamaño es de 64
entradas, ya que los mismos ocupan 46 y 53 páginas. En este caso, los fallos de paginación se dan 
solo la primera vez que se accede a una página y nunca es necesario sacar una entrada de la TLB para 
que entre otra nueva. En cambio, si el tamaño de la TLB es de 32 entradas, sí se producen más fallos 
de paginación y nuevas entradas en la TLB en ocasiones tienen que reemplazar a otras existentes.
Sin embargo, vemos que para este tipo de programas el porcentaje de aciertos es muy alto tanto para
una TLB de 32 como de 64, por lo que considero que ambos tamaños son adecuados.

Se modifica el constructor y el método LoadPageInTlb de la clase AddrSpace para implementar carga 
por demanda pura. Se agrega un booleano llamado "init" en la entrada de la tabla de paginación para 
indicar si la página está inicializada o no. En el constructor del AddrSpace no se inicializa 
ninguna página. Cuando una página es cargada a la TLB por primera vez en el método LoadPageInTlb, 
se inicializa la página copiando los datos desde el ejecutable si corresponde.

Se agrega la bandera "v" para imprimir información de debug relacionada a memoria virtual y 
paginación.

Para implementar paginación, se crea la clase CoreEntry en "coremap.h" y una lista global de tipo 
SynchList para el coremap, para llevar registro de a qué página virtual de qué address space 
corresponde cada página física. En el constructor de AddrSpace se crea un archivo de paginación y se
agrega el OpenFile llamado swap a sus propiedades. Se agrega un booleano llamado "disk" en la 
entrada de la tabla de paginación para indicar cuando la página no está mapeada a la memoria 
principal sino que hay que ir a buscarla al archivo swap del disco. Se agregan los métodos 
SaveToDisk y RestoreFromDisk a la clase AddrSpace para guardar y restaurar una página. 

Se implementan las políticas de paginación FIFO y LRU. Si se ejecuta nachos con la opción -lru se 
utilizará la política de paginación LRU. Caso contrario, se utiliza FIFO.

En ambas políticas, cuando se requiere una nueva página física para restaurar una página virtual 
desde el disco se toma la primer entrada del frente de la lista coremap y luego de utilizarla se
la coloca al final de la lista. Para la política LRU, cada vez que se realiza un acceso a memoria, 
la entrada en el coremap correspondiente a su página física se la translada al final, quedando en 
el frente siempre la página con acceso menos reciente. Es una implementación rudimentaria y bastante
lenta pero para el propósito de las pruebas fue útil.

Para las siguientes pruebas se redujo el número de páginas físicas a 16. De esta manera los 
programas no entran por completo en la memoría principal y se tienen que grabar páginas en disco.

$ ../vm/nachos -x sort
Page hit: 44.067.596, page miss: 19.582
Page to disk: 16.118, page from disk: 19.543

$ ../vm/nachos -x sort -lru
Page hit: 44.066.961, page miss: 17.043
Page to disk: 14.964, page from disk: 14.941

$ ../vm/nachos -x matmult
Page hit: 716.044, page miss: 8.160
Page to disk: 1.008, page from disk: 8.114

$ ../vm/nachos -x matmult -lru
Page hit: 715.185, page miss: 6.450
Page to disk: 55, page from disk: 5.207

Se puede ver claramente que con la política LRU se producen menos accesos a disco que con la 
política FIFO. "Page to disk" indica las veces que se tuvo que grabar una página a disco y 
"Page from disk" las veces que se trajo una página desde el disco a la memoria principal. Notar que
la cantidad de grabaciones al disco es menor que las lecturas ya que si la página en memoria está 
limpia (no tuvo ningún cambio) no se necesita grabar nuevamente al disco al retirarla.

Offtopic:
Se relizó la siguiente modificación que corresponde a una práctica anterior. En el scheduler, en 
lugar de usar un List, Lock y Condition para la lista de estados de salida se pasa a utilizar un 
SynchList que ya está sincronizada y produce el mismo resultado.

En resumen, los archivos modificados en esta práctica fueron los siguientes:
- machine/machine.cc
- machine/machine.h
- machine/stats.cc
- machine/stats.h
- machine/translate.cc
- machine/translate.h
- threads/system.cc
- threads/system.h
- threads/thread.cc
- userprog/addrspace.cc
- userprog/addrspace.h
- userprog/exception.cc
- userprog/progtest.cc
- vm/coremap.h

-----------------------

Alumno: Ait, Ismael
