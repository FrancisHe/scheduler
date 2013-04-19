/**
 * a memory-managed round-robin dispatcher
 * author: hwx
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/wait.h>
#include	"common.h"

static PcbPtr inputQ = NULL;	// input queue
static PcbPtr userJobQ = NULL;	// user job queue
static PcbPtr readyQ = NULL;	// ready queue

void runRR();

int
main(int argc, char *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s <input_file>\n", argv[0]);
		exit(-1);
	}
	if (!readInput(argv[1], &inputQ))
	{
		fprintf(stderr, "read input file error\n");
		exit(-1);
	}
//	printInputQ(inputQ);
	runRR();

	return 0;
}

/**
description:
	run the round-robin algorithm
*/
void
runRR()
{
	PcbPtr currentProcess = NULL;
	PcbPtr tmpPcb = NULL;
	PcbPtr tmpPcb2 = NULL;
	MabPtr tmpMab = NULL;
	int dispatcherTimer = 0;
	
	memInit();
	while (inputQ || readyQ || userJobQ || currentProcess)
	{
//		printf("%d\n", (dispatcherTimer + 1));
		while ( inputQ && (inputQ->arrivalTime <= dispatcherTimer) )
		{
			tmpPcb = deqPcb(&inputQ);
			userJobQ = enqPcb(userJobQ, tmpPcb);
		}

		tmpPcb = userJobQ;
		while (tmpPcb)
		{
			/* delist only one process */
			if ((tmpMab = memChk(tmpPcb->memAllocSize, NEXT_FIT)) != NULL)
			{
				tmpPcb->memBlock = memAlloc(tmpMab, tmpPcb->memAllocSize);
				/*printf("allocate\n");
				printMemTrace();
				printf("-------\n");*/
				if (tmpPcb == userJobQ)
				{
					tmpPcb2 = deqPcb(&userJobQ);
					readyQ = enqPcb(readyQ, tmpPcb2);
				}
				else
				{
					tmpPcb2 = tmpPcb->next;
					readyQ = enqPcb(readyQ, tmpPcb);
				}
				break;
			}
			tmpPcb2 = tmpPcb;
			tmpPcb = tmpPcb->next;
		}

		if (!currentProcess && readyQ)
		{
			currentProcess = deqPcb(&readyQ);
			startPcb(currentProcess);
			goto NEXT;
		}

		if (currentProcess)
		{
			currentProcess->remainingCpuTime--;
			if (currentProcess->remainingCpuTime <= 0)
			{
				terminatePcb(currentProcess);
				waitpid(currentProcess->pid, NULL, WUNTRACED);
				freePcb(currentProcess);
				currentProcess = NULL;
			}
			else
			{
				if (readyQ)
				{
					suspendPcb(currentProcess);
					waitpid(currentProcess->pid, NULL, WUNTRACED);
					readyQ = enqPcb(readyQ, currentProcess);
					currentProcess = NULL;
				}
			}
			if (readyQ)
			{
				currentProcess = deqPcb(&readyQ);
				startPcb(currentProcess);
			}
		}

	NEXT:
		sleep(1);
		dispatcherTimer++;
	}
}
