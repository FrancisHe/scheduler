#ifndef	_COMMON_H
#define	_COMMON_H

#include	<unistd.h>

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	FALSE
#define	FALSE	0
#endif

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

#endif	/* _COMMON_H */
