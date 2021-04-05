En la clase AddrSpace:
    Se agrega método LoadPageInTlb para cargar una nueva entrada en la tabla de paginación 
    descartando la más vieja (FIFO).

    Dentro del método RestoreState se inicializa la TLB poniendo como no válidas todas sus entradas.

    Se agregan métodos ReadMem y WriteMem que reintentan la lectura/escritura si se está utilizando 
    TLB. Estos métodos solo se usarán desde el sistema en modo kernel. En modo usuario, el reintento
    se produce por no aumentar el program counter.

En exception.cc se agrega el manejo de la excepción PageFaultException que hace que se cargue la 
entrada de la página correspondiente en la TLB.

Se arregla el programa de usuario sort.c que no estaba ordenando bien el array. Además el resultado 
que se espera luego de ordenar el array en forma ascendente es 1 y no de 0 como indicaba el 
comentario, ya que 1 es el menor de los valores que se guarda en el array.

Al ejecutar el programa "sort" se obtuvieron los siguientes resultados
Con TLBSize=32, Page hit: 44.056.119, page miss: 5.115, porcentaje de aciertos: 99,9883912%
Con TLBSize=64, Page hit: 44.051.505, page miss:    39, porcentaje de aciertos: 99,9999115%
Cantidad de páginas totales de su espacio de direcciones: 46 pages

Al ejecutar el programa "matmult" se obtuvieron los siguientes resultados
Con TLBSize=32, Page hit: 709.417, page miss: 106, porcentaje de aciertos: 99,9850604%
Con TLBSize=64, Page hit: 709.365, page miss:  46, porcentaje de aciertos: 99,9935157%
Cantidad de páginas totales de su espacio de direcciones: 53 pages

Vemos que claramente al aumentar el tamaño de la TLB se producen más aciertos de páginas. De hecho, 
las tablas de paginación de ambos programas entran completamente en la TLB cuando su tamaño es de 64
entradas, ya que los mismos ocupan 46 y 53 páginas. En este caso, los fallos de paginación se dan 
solo la primera vez que se accede a una página y nunca es necesario sacar una entrada de la TLB para 
que entre otra nueva. En cambio, si el tamaño de la TLB es de 32 entradas, sí se producen más fallos 
de paginación y nuevas entradas en la TLB en ocaciones tienen que reemplazar a otras existentes.
Sin embargo, vemos que para este tipo de programas el porcentaje de aciertos es muy alto tanto para
una TLB de 32 como de 64, por lo que considero que ambos tamaños son adecuados.

Se implementa carga por demanda pura en AddrSpace. Se agrega un booleano en la entrada de la tabla 
de paginación para indicar si la página está inicializada o no. En el constructor del AddrSpace no 
se inicializa ninguna página. Cuando una página es cargada a la TLB por primera vez en el método 
"LoadPageInTlb", se inicializa la página copiando los datos desde el ejecutable si corresponde.

En lugar de usar un List, Lock y Condition en el scheduler para la lista de estados de salida
pasé a utilizar un SynchList ya que el resultado es exactamente el mismo.

Se agrega la clase CoreEntry en "coremap.h" y una lista global de tipo SynchList para el coremap, 
para llevar registro de a qué página virtual de qué address space corresponde cada página física.

Se agrega un OpenFile llamado swap al AddrSpace para paginación.

Con la bandera de debug "e" se imprime el estado de salida de los procesos al hacer la llamada a 
sistema Exit. Con la bandera "v" se imprime información de memoria virtual.

