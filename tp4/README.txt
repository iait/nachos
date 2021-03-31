Práctica 4
-----------

Se agregó la bandera de DEBUG 'u' para mostrar lo referente a la práctica de userprog y las llamadas
a sistema de nachos.

Se aumentó la memoria física llevando el número de páginas de 32 a 128.

Se crearon varios programas de usuarios para test:
 - exit.c
 - create.c
 - read.c
 - console.c 
 - printA.c 
 - printB.c
 - exec.c 
 - args.c
 - print.c
 - echo.c
 - touch.c

Se agregó la posibilidad de pasar varios programas de usuario para ejecutar simultaneamente por 
línea de comandos al ejecutar nachos con la opción -x. Por ejemplo, el programa printA imprime en 
consola 10 letras A y el programa printB imprime 10 letras B. Entonces se puede invocar a nachos 
para que corran ambos programas concurrentemente con:
$ ./nachos -x ../test/printA ../test/printB -rs 1
La salida es similar a la siguiente: ABBBBABBAAAABABBBAAA
Debido a este cambio, el hilo "main" ya no ejecuta el programa de usuario sino que crea un hilo y 
hace Fork por cada programa de usuario que se le pase como argumento a -x.
Esto permitió una primera prueba de las implementación de multiprogramación antes de implementar la
llamada a sistema "exec".
Luego de implementar dicha llamada a sistema se creó un programa de usuario llamado exec.c que lanza
esos dos programas.
De esta manera, las dos ejecuciones mostradas a continuación son similares en su funcionamiento:
$ ./nachos -x ../test/printA ../test/printB -rs 1
$ ./nachos -x ../test/exec -rs 1

En la implementación de listas en "list.h" se agregaron métodos para utilizarla como un mapa de 
clave-valor aprovechando que la clase ListElement ya tiene una propiedad entera "key" que se usa 
para la clave. Se agregaron métodos para insertar un elemento con clave al final de la lista y uno 
para extraer un elemento a partir de su clave, que hace búsqueda lineal. Se utiliza en Scheduler 
para asociar los spaceId de los programas finalizados con su estado de finalización.

Se unifica el join de la práctica anterior con el de esta práctica. Como ahora los hilos tienen 
identificador, en lugar de guardar la lista de los hilos que se van a hacer join en el scheduler, 
se guarda una lista con los SpaceId y su estado de finalización asociado. El hilo entonces se puede 
limpiar cuando termina sin problemas.

Se agrega a Thread una tabla de file descriptors con un método para agregar un archivo y retornar su 
file descriptor, un método para obtener un archivo abierto a partir de su file descriptor y un 
método para eliminar una entrada de la tabla al cerrar el archivo. Los file descriptors 0 y 1 quedan
reservados para la entrada y salida estándar de consola y no se permite que se cierren.

Se agrega synchconsole.h y synchconsole.cc para implementar SynchConsole con el acceso sincronizado 
a la consola similar a como está implementado SynchDisk. Además se agrega la opción -sc para 
probar la consola sincronizada con un simple test en progtest.cc llamado SynchConsoleTest, igual al 
ConsoleTest pero con la consola que ya está sincronizada en lugar de la original. Más abajo en este 
README se detalla un problema que se encontró con la consola al ejecutar por ejemplo el programa de 
usuario shell.c y cómo fue corregido.

Se agrega un bitmap al address space para implementar tablas de paginación y multiprogramación. Se
agrega el método CopyToAddrSpace que graba en memoria las secciones del ejecutable teniendo en 
cuenta la tabla de paginación.

En "progtest.cc" la función StartUserProgram luego de crear el espacio de direcciones e inicializar 
los registros, graba en el fondo del stack los argumentos con los que se ejecuta el programa y
guarda en los registros r4 y r5 los valores de argc y argv respectivamente para que los reciba el 
main del programa de usuario. Luego se inicializa el stack pointer.

Resumen de los archivos que se crearon y modificaron para esta práctica:
- threads/main.cc
- thread/thread.h
- thread/thread.cc
- threads/list.h
- thread/scheduler.h
- thread/scheduler.cc
- thread/system.h
- thread/system.cc
- threads/threadtest.cc
- userprog/progtest.cc
- userprog/synchconsole.h
- userprog/synchconsole.cc
- userprog/exception.cc
- userprog/addrspace.h
- userprog/addrspace.cc
- machine/interrupt.cc
- machine/machine.h
- machine/machine.cc
- machine/stats.h

--------------------------------

Problema con la consola:

Se detectó un problema con la consola cuando se espera a que se ingresen datos en la misma y no hay 
ningún proceso disponible para ejecutar. Por ejemplo, si ejecuto el programa de usuario "shell", 
éste se bloquea esperando a que haya algún caracter disponible en la consola. Al ser el único 
programa iniciado y estar bloqueado, la lista readyList del scheduler queda vacía. En el método 
Sleep de la clase Thread donde se bloquea al hilo del programa "shell" hay un while que busca el 
próximo hilo a ejecutar y mientras no haya más hilos para ejecutar, como es este el caso, llama al 
manejador de interrupciones para ver si se puede despertar a alguno.

Esto hace que se invoque a Interrupt::Idle y luego a Interrupt::CheckIfDue y se adelante el reloj 
100 ticks (donde se encuentra la interrupción que revisa entrada en consola). Como todavía no hay 
ningún caracter disponible, no se despierta a ningún programa y se vuelve a repetir todo el proceso
desde el while de Sleep, adelantando el reloj 100 ticks repetidamente.

Esto hace que se entre en un busy-waiting esperando por entrada en consola y que el contador de 
ticks, que está definido como entero, se aumente rápidamente cada 100 ticks hasta que dé overflow.
Cuando da overflow la próxima interrupción que se programa se lo hace con un tiempo "menor" a la 
anterior.

Ejemplo que obtuve depurando luego de unos segundos de levantar nachos ejecutando el programa shell:
* tiempo actual (stats->totalTicks) 2147483569
* tiempo en el que se programa la próxima interrupción de consola (when) -2147483640
Notar que el tiempo when es igual al totalTicks más 100 luego de dar overflow de int.

A partir de este momento como el when es menor al actual, se maneja esa interrupción lo que provoca 
que se genere otra para el mismo momento y entra en un ciclo infinito en el while de Interrupt::Idle
 while (CheckIfDue(false)) ;
Esto traba toda la ejecución del programa. Ahora por más que ingrese algo en la consola, no se 
avanzará de ese lugar.

Para solucionarlo hice lo siguiente:

Primero, agregué un ASSERT para evitar que el contador de ticks dé overflow. Luego, en lugar de 
adelantar inmediatamente el reloj a la próxima interrupción si no hay programas de usuario para 
ejecutar, puse un sleep proporcional a la cantidad de ticks que faltan para la próxima interrupción.
Establecí un tiempo de 10 microsegundos por tick. De esta manera se evita el busy-waiting y no se 
avanza el contador de ticks tan rápido mientras se espera el ingreso por consola, evitando que dé 
overflow.

Con este arreglo, en teoría nachos podría estar ejecutandose de forma continua por aprox 6hs horas 
a la espera de un ingreso por consola antes de que el contador de ticks dé overflow (en realidad, 
antes de que falle por el ASSERT).
(2^31-1) * 10 microsegundos = 6 hs

En resumen:

Se definió en stats.h
const int usecondPerTick = 10;

Se agregó en interrupt.cc, en clase Interrupt método CheckIfDue
    if (advanceClock && when > stats->totalTicks) { // advance the clock
        unsigned int diffTicks = when - stats->totalTicks;
        usleep(diffTicks * usecondPerTick); // espera antes de avanzar el reloj
        stats->totalTicks = when;
    }

-----------------------

Alumno: Ait, Ismael