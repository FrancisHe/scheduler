#ifndef COMMON_H
#define COMMON_H

#define MAXARGS 2
#define MAXMEM 1024	
#include <stdlib.h>
#include <stdio.h>

struct mab {
    int offset;
    int size;
    int allocated;
    struct mab * next;
    struct mab * prev;
};
typedef struct mab Mab;
typedef Mab * MabPtr;

struct pcb {
    pid_t pid;             // system process ID
    char * args[MAXARGS];  // program name and args
    int arrivaltime;
    int remainingcputime;
    int priority;
    int memory_alloc;
    struct pcb * next;     // links for Pcb handlers
    MabPtr memblock;
};
typedef struct pcb Pcb;
typedef Pcb * PcbPtr;	


void error(void);
void print_arrivaltimes(PcbPtr head);
int read_input(FILE * file, PcbPtr * head);


PcbPtr startPcb(PcbPtr process);
PcbPtr terminatePcb(PcbPtr process);
PcbPtr createnullPcb(void);
PcbPtr enqPcb (PcbPtr headofQ, PcbPtr process);
PcbPtr deqPcb (PcbPtr * headofQ);


MabPtr memChk(MabPtr m, int size);   // check if memory available
MabPtr memAlloc(MabPtr m, int size); // allocate memory block
MabPtr memFree(MabPtr m);            // free memory block
MabPtr memMerge(MabPtr m);           // merge two memory blocks
MabPtr memSplit(MabPtr m, int size); // split a memory block

#endif
