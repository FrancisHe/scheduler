#include	<stdio.h>
#include	<string.h>
#include	<errno.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<sys/wait.h>
#include	"common.h"


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
