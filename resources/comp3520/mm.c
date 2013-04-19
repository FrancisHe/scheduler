/*************************************************
//A memory-managed feedback dispatcher
//Evan Pollack 307278263
*************************************************/
#include "common.h"
#include "mm.h"
#include <signal.h>
Pcb * input_head = NULL;
Pcb * rq0_head = NULL;
Pcb * rq1_head = NULL;
Pcb * rq2_head = NULL;
Pcb * ujq_head = NULL;

MabPtr memory = NULL;
MabPtr next_fit_ptr = NULL;


int
main(int argc, char** argv){
	if(argc < 1)
		error();
	freopen(argv[1],"r",stdin);
	read_input(stdin,&input_head);
	memory = malloc(sizeof(Mab));
	memory->next = memory;
	memory->prev = memory;
	memory->allocated = 0;
	memory->size = MAXMEM;
	memory->offset = 0;
	next_fit_ptr = memory;
	mm_run();
	return EXIT_SUCCESS;
}

//funtion to run the algorithm
void
mm_run(){
	PcbPtr current_process = NULL;
	int dispatch_timer = 0;
	while(input_head || rq0_head || rq1_head ||rq2_head || ujq_head || current_process){
	//	printf("=========after %d seconds==========\n",dispatch_timer);
	//	puts("input q:");
	///	print_arrivaltimes(input_head);
	//	puts("rq0 q:");
	//	print_arrivaltimes(rq0_head);
	//	puts("rq1 q:");
	//	print_arrivaltimes(rq1_head);
	//	puts("rq2 q:");
	//	print_arrivaltimes(rq2_head);
	//	puts("=====================================");
		while(input_head && input_head->arrivaltime <= dispatch_timer){
			//dequeue the head of the input queue, then add
			//it to the end of the round robin queue.
			//We need to initialize the head if we haven't already
			if(!ujq_head){
				ujq_head = deqPcb(&input_head);
				ujq_head->next = NULL;
			}
			else{
				PcbPtr tmp = deqPcb(&input_head);
				tmp->next = NULL;
				enqPcb(ujq_head,tmp);
			}
		}
		while(ujq_head && memChk(next_fit_ptr,ujq_head->memory_alloc)){
			ujq_head->memblock = memAlloc(next_fit_ptr,ujq_head->memory_alloc);
			//set the next fit pointer to the block of memory after what we
			//just malloced.
			next_fit_ptr = ujq_head->memblock->next;
			if(!rq0_head){
				rq0_head = deqPcb(&ujq_head);
				rq0_head->next = NULL;
			}
			else{
				PcbPtr tmp = deqPcb(&ujq_head);
				tmp->next = NULL;
				enqPcb(rq0_head,tmp);
			}
		}
		if(current_process){
			current_process->remainingcputime--;
			if(current_process->remainingcputime <= 0){
				terminatePcb(current_process);
				memFree(current_process->memblock);
				free(current_process);
				current_process = NULL;
				
			}
			//if there is something waiting in the RR queue,
			//we need to stop the current process and give
			//another process some time.
			else if(rq0_head || rq1_head || rq2_head){
				if(kill(current_process->pid,SIGTSTP)){
					fprintf(stderr,"temp stop of %d failed",
							(int)current_process->pid);
				}
				//a switch holds no advantage here over if/else,
				//however it would be nicer if we had more feedback
				//queues.
				switch(current_process->priority){
					case 1:
						current_process->priority = 2;
						if(!rq1_head)
							rq1_head = current_process;
						else
							enqPcb(rq1_head,current_process);
						break;
					default:
						current_process->priority = 3;
						if(!rq2_head)
							rq2_head = current_process;
						else
							enqPcb(rq2_head,current_process);
						break;
				}
				current_process = NULL;
			}
		}
		if(!current_process){
			//make sure we don't mix up output from the previous
			//stop/term before the start/cont!
			waitpid(-1, NULL, WUNTRACED);
			if(rq0_head){
				current_process = deqPcb(&rq0_head);
				current_process->next = NULL;
				startPcb(current_process);
			}
			else if(rq1_head){
				current_process = deqPcb(&rq1_head);
				current_process->next = NULL;
				startPcb(current_process);
			}
			else if(rq2_head){
				current_process = deqPcb(&rq2_head);
				current_process->next = NULL;
				startPcb(current_process);
			}
		}
		++dispatch_timer;
		sleep(1);
	}

}
