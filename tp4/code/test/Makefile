# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
#GCCDIR = ~/gnu/local/decstation-ultrix/bin/
#GCCDIR = ../../../usr/local/decstation-ultrix/bin/
GCCDIR = /usr/local/mips/bin/
LDFLAGS = -T script -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)


# if you aren't cross-compiling:
#GCCDIR =
#LDFLAGS = -N -T 0
#ASFLAGS =
#CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)mips-gcc
AS = $(GCCDIR)mips-as
LD = $(GCCDIR)mips-ld

CPP = /lib/cpp
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR) -mips1

all: halt shell matmult sort exit create read console printA printB exec

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt
	
exit.o: exit.c
	$(CC) $(CFLAGS) -c exit.c
exit: exit.o start.o
	$(LD) $(LDFLAGS) start.o exit.o -o exit.coff
	../bin/coff2noff exit.coff exit
	
create.o: create.c
	$(CC) $(CFLAGS) -c create.c
create: create.o start.o
	$(LD) $(LDFLAGS) start.o create.o -o create.coff
	../bin/coff2noff create.coff create
	
read.o: read.c
	$(CC) $(CFLAGS) -c read.c
read: read.o start.o
	$(LD) $(LDFLAGS) start.o read.o -o read.coff
	../bin/coff2noff read.coff read

console.o: console.c
	$(CC) $(CFLAGS) -c console.c
console: console.o start.o
	$(LD) $(LDFLAGS) start.o console.o -o console.coff
	../bin/coff2noff console.coff console

printA.o: printA.c
	$(CC) $(CFLAGS) -c printA.c
printA: printA.o start.o
	$(LD) $(LDFLAGS) start.o printA.o -o printA.coff
	../bin/coff2noff printA.coff printA

printB.o: printB.c
	$(CC) $(CFLAGS) -c printB.c
printB: printB.o start.o
	$(LD) $(LDFLAGS) start.o printB.o -o printB.coff
	../bin/coff2noff printB.coff printB

exec.o: exec.c
	$(CC) $(CFLAGS) -c exec.c
exec: exec.o start.o
	$(LD) $(LDFLAGS) start.o exec.o -o exec.coff
	../bin/coff2noff exec.coff exec

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult


# Estas reglas sirven para compilar programas simples,
# que consistan en un �nico fuente.
# Las reglas anteriores para construir los ejecutables
# halt, shell, sort y matmult se podr�an suprimir

%: %.c 
	make start.o 
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) start.o $*.o -o $*.coff
	../bin/coff2noff $*.coff $@
	

