#ifndef	_COMMON_H
#define	_COMMON_H

#include	<unistd.h>

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FALSE
#define	FALSE	0
#endif

struct mab
{
	int offset;
	int size;
	int allocated;
	struct mab *next;
	struct mab *prev;
};
typedef struct mab Mab;
typedef Mab *MabPtr;

#define	TOTAL_MEM	1024

/* memory allocation policy */
enum
{
	FIRST_FIT = 1,
	NEXT_FIT = 2,
	BEST_FIT = 3,
	WORST_FIT = 4
};

extern MabPtr memStart;
extern MabPtr memNext;

#define	MAXARGS	2

/* process status */
enum
{
	DEFAULT = 0,
	SUSPENDED = 1,
};

struct pcb
{
	pid_t pid;				// process ID
	char *args[MAXARGS];	// program name and args
	int arrivalTime;
	int remainingCpuTime;
	int status;
	int memAllocSize;
	MabPtr memBlock;
	struct pcb *next;
};

typedef struct pcb Pcb;
typedef Pcb *PcbPtr;

extern int readInput(const char *file, PcbPtr *inputQ);
extern void printInputQ(PcbPtr inputQ);
extern PcbPtr endofQ(PcbPtr headofQ);
extern void freePcb(PcbPtr pcbPtr);
extern PcbPtr createnullPcb();
extern PcbPtr restartPcb(PcbPtr process);
extern PcbPtr startPcb(PcbPtr process);
extern PcbPtr terminatePcb(PcbPtr process);
extern PcbPtr suspendPcb(PcbPtr process);
extern PcbPtr enqPcb(PcbPtr headofQ, PcbPtr process);
extern PcbPtr deqPcb(PcbPtr *headofQ);

extern MabPtr memInit();					// initialize memory
extern MabPtr memChk(int size, int policy);	// check if memory available
extern MabPtr memAlloc(MabPtr m, int size);	// allocate memory block
extern MabPtr memFree(MabPtr m);		// free memory block
extern MabPtr memMerge(MabPtr m);			// merge two memory blocks
extern void printMemTrace();

#endif	/* _COMMON_H */
