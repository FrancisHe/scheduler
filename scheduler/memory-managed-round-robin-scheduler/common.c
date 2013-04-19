#include	<stdio.h>
#include	<string.h>
#include	<errno.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<sys/wait.h>
#include	"common.h"

MabPtr memStart = NULL;
MabPtr memNext = NULL;

/**
description:
	read from input file.
	file - path of input file
	inputQ - head of input queue
returns:
	TRUE on success, and ... inputQ ...
	FALSE if failed
*/
int
readInput(const char *file, PcbPtr *inputQ)
{
	FILE *fp = NULL;
	int arrivalTime, priority, cputime, memoryAlloc, res1, res2, res3, res4;
	PcbPtr newPcb = NULL;
	PcbPtr pre = NULL;
	
	fp = fopen(file, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "fopen error: %p\n", strerror(errno));
		return FALSE;
	}
	
	*inputQ = NULL;
	while (fscanf(fp, "%d, %d, %d, %d, %d, %d, %d, %d", &arrivalTime, &priority, &cputime, &memoryAlloc, &res1, &res2, &res3, &res4) == 8)
	{
		newPcb = createnullPcb();
		newPcb->pid = 0;
		newPcb->args[0] = "./process";
		newPcb->args[1] = NULL;
		newPcb->arrivalTime = arrivalTime;
		newPcb->remainingCpuTime = cputime;
		newPcb->status = DEFAULT;
		newPcb->memAllocSize = memoryAlloc;
		newPcb->memBlock = NULL;
		newPcb->next = NULL;
		
		if (*inputQ == NULL)
		{
			*inputQ = newPcb;
		}
		else
		{
			pre->next = newPcb;
		}
		pre = newPcb;
	}
	if (!feof(fp))
	{
		goto FAILURE;
	}
	
	fclose(fp);
	return TRUE;
FAILURE:
	fclose(fp);
	return FALSE;
}

void
printInputQ(PcbPtr inputQ)
{
	int i;

	while (inputQ)
	{
		fprintf(stdout, "Pcb.pid = %d\n", inputQ->pid);
		for (i = 0; inputQ->args[i]; i++)
		{
			fprintf(stdout, "%s ", inputQ->args[i]);
		}
		fprintf(stdout, "\n");
		fprintf(stdout, "Pcb.arrivalTime = %d\n", inputQ->arrivalTime);
		fprintf(stdout, "Pcb.remainingCpuTime = %d\n", inputQ->remainingCpuTime);
		fprintf(stdout, "Pcb.status = %d\n", inputQ->status);
		fprintf(stdout, "\n");
		inputQ = inputQ->next;
	}
}

/**
returns:
	PcbPtr of end of the queue
*/
PcbPtr
endofQ(PcbPtr headofQ)
{
	PcbPtr tmpPcb;

	if (headofQ == NULL)
	{
		return NULL;
	}
	tmpPcb = headofQ;
	while (tmpPcb->next)
	{
		tmpPcb = tmpPcb->next;
	}
	return tmpPcb;
}

/**
description:
	free a Pcb
*/
void
freePcb(PcbPtr pcbPtr)
{
	int i;
	
	for (i = 0; pcbPtr->args[i]; i++)
	{
		pcbPtr->args[i] = NULL;
	}
	free(pcbPtr);
}

/**
description:
	create inactive Pcb
returns:
	PcbPtr of newly initialised Pcb
	NULL if malloc failed
*/
PcbPtr
createnullPcb()
{
	return (PcbPtr) malloc(sizeof(Pcb));
}

/**
description:
	restart a suspended process
returns:
	PcbPtr of process
	NULL if start failed
*/
PcbPtr
restartPcb(PcbPtr process)
{
	if (kill(process->pid, SIGCONT) < 0)
	{
		fprintf(stderr, "kill(SIGCONT) error: %p\n", strerror(errno));
		return NULL;
	}
	process->status = DEFAULT;
	return process;
}

/**
description:
	start a process
returns:
	PcbPtr of process
	NULL if start failed
*/
PcbPtr
startPcb(PcbPtr process)
{
	PcbPtr tmpPcb = NULL;

	if (process->status == SUSPENDED)
	{
		tmpPcb = restartPcb(process);
		waitpid(tmpPcb->pid, NULL, WCONTINUED);
		return tmpPcb;
	}
	
	switch (process->pid = fork())
	{
	case -1:	// error
		fprintf(stderr, "fork error: %p\n", strerror(errno));
		break;
	case 0:		// child
		execvp(process->args[0], process->args);
		fprintf(stderr, "execvp error: %p\n", strerror(errno));	// can't reach here if succeed
		break;
	default:	// parent
		process->status = DEFAULT;
		return process;
	}
	
	return NULL;
}

/**
description:
	terminate a process
returns:
	PcbPtr of process
	NULL if terminate failed
*/
PcbPtr
terminatePcb(PcbPtr process)
{
	if (kill(process->pid, SIGINT) < 0)
	{
		fprintf(stderr, "kill(SIGINT) error: %p\n", strerror(errno));
		return NULL;
	}
	
	MabPtr tmpMab = NULL;
	tmpMab = memFree(process->memBlock);
	/*printf("free\n");
	printMemTrace();
	printf("-------\n");*/
	return process;
}

/**
description:
	suspend a running process
returns:
	PcbPtr of process
	NULL if terminate failed
*/
PcbPtr
suspendPcb(PcbPtr process)
{
	if (kill(process->pid, SIGTSTP) < 0)
	{
		fprintf(stderr, "kill(SIGTSTP) error: %p\n", strerror(errno));
		return NULL;
	}
	
	process->status = SUSPENDED;
	return process;
}

/**
description:
	queue process (or join queues) at end of queue
	enqueues at "tail" of queue list.
returns:
	(new) head of queue
*/
PcbPtr
enqPcb(PcbPtr headofQ, PcbPtr process)
{
	PcbPtr end = NULL;
	
	process->next = NULL;
	if (headofQ == NULL)
	{
		headofQ = process;
	}
	else
	{
		end = endofQ(headofQ);
		end->next = process;
	}
	return headofQ;
}


/**
description:
	dequeue process - take Pcb from "head" of queue.
returns:
	PcbPtr if dequeued,
	NULL if queue was empty
	& sets new head of Q pointer in adrs at 1st arg
*/
PcbPtr
deqPcb(PcbPtr *headofQ)
{
	PcbPtr process = NULL;

	if (*headofQ == NULL)
	{
		return NULL;
	}
	process = *headofQ;
	*headofQ = (*headofQ)->next;
	return process;
}

/**
description:
	initialize memory
returns:
	on success, pointer to the initialized Mab will be returned, NULL on failure.
*/
MabPtr
memInit()
{
	MabPtr newMab = NULL;

	newMab = (MabPtr)malloc(sizeof(Mab));
	if (newMab == NULL)
	{
		return NULL;
	}
	newMab->allocated = FALSE;
	newMab->offset = 0;
	newMab->size = TOTAL_MEM;
	newMab->prev = NULL;
	newMab->next = NULL;

	memStart = newMab;
	memNext = memStart;
	return newMab;
}

/**
description:
	check if memory available.
	policy - memory allocation policy
returns:
	pointer to a available memory block on success, NULL on failure.
*/
MabPtr
memChk(int size, int policy)
{
	MabPtr tmpMab = NULL;
	switch (policy)
	{
	case FIRST_FIT:	// fisrt fit
		break;
	case BEST_FIT:	// best fit
		break;
	case WORST_FIT:	// worst fit
		break;
	default:		// next fit
		tmpMab = memNext;
		do
		{
			if (!tmpMab->allocated && size <= tmpMab->size)
			{
				return tmpMab;
			}
			tmpMab = tmpMab->next;
			if (tmpMab == NULL)	// reach end of the linked list
			{
				tmpMab = memStart;
			}
		} while (tmpMab != memNext);
		break;
	}
	return NULL;
}

/**
description:
	allocate memory block.
	m - the target unallocated memory block.
returns:
	on success, pointer to the allocated memory block will be returned. NULL on failure.
*/
MabPtr
memAlloc(MabPtr m, int size)
{
	MabPtr newMab = NULL;

	m->allocated = TRUE;
	if (size == m->size)
	{
		/* update memNext */
		if (m->next == NULL)
		{
			memNext = memStart;
		}
		else
		{
			memNext = m->next;
		}
		return m;
	}

	newMab = (MabPtr)malloc(sizeof(Mab));
	if (newMab == NULL)
	{
		return NULL;
	}
	newMab->allocated = FALSE;
	newMab->offset = m->offset + size;
	newMab->size = m->size - size;
	newMab->prev = m;
	newMab->next = m->next;

	memNext = newMab;	// update memNext

	if (m->next)	// update m->next->prev
	{
		m->next->prev = newMab;
	}

	/* update m */
	m->size = size;
	m->next = newMab;

	return m;
}

/**
description:
	free memory block with merging the previous or next memory blocks.
	m - the target memory block.
returns:
	(new) m
*/
MabPtr
memFree(MabPtr m)
{
	m->allocated = FALSE;
	while (m->next && !m->next->allocated)
	{
		m = memMerge(m);
	}
	while (m->prev && !m->prev->allocated)
	{
		m = memMerge(m->prev);
	}
	return m;
}

/**
description:
	merge m and m->next
returns:
	(new) m
*/
MabPtr
memMerge(MabPtr m)
{
	MabPtr tmpMab = NULL;

	tmpMab = m->next;
	m->size += tmpMab->size;
	m->next = tmpMab->next;
	if (tmpMab->next)
	{
		tmpMab->next->prev = m;
	}
	if (memNext == tmpMab)	// update memNext
	{
		memNext = m;
	}
	free(tmpMab);
	return m;
}


void
printMemTrace()
{
	MabPtr tmpMab = NULL;

	tmpMab = memStart;
	while (tmpMab)
	{
		if (tmpMab->allocated)
		{
			printf("allocated = yes, ");
		}
		else
		{
			printf("allocated = no, ");
		}
		printf("size = %d\n", tmpMab->size);
		tmpMab = tmpMab->next;
	}
}
