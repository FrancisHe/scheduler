/*************************************************
//A feedback dispatcher
//Evan Pollack 307278263
*************************************************/
#include "common.h"
#include "fd.h"
#include <signal.h>
Pcb * input_head = NULL;
Pcb * rq0_head = NULL;
Pcb * rq1_head = NULL;
Pcb * rq2_head = NULL;


int
main(int argc, char** argv){
	//we want to be able to take both file and fstreams
	if(argc < 1)
		error();
	freopen(argv[1],"r",stdin);
	read_input(stdin,&input_head);
	fd_run();
	return EXIT_SUCCESS;
}

//funtion to run the algorithm
void
fd_run(){
	PcbPtr current_process = NULL;
	int dispatch_timer = 0;
	while(input_head || rq0_head || rq1_head ||rq2_head || current_process){
		while(input_head && input_head->arrivaltime <= dispatch_timer){
			//dequeue the head of the input queue, then add
			//it to the end of the round robin queue.
			//We need to initialize the head if we haven't already
			if(!rq0_head){
				rq0_head = deqPcb(&input_head);
				rq0_head->next = NULL;
			}
			else{
				PcbPtr tmp = deqPcb(&input_head);
				tmp->next = NULL;
				enqPcb(rq0_head,tmp);
			}
		}
		if(current_process){
			current_process->remainingcputime--;
			if(current_process->remainingcputime <= 0){
				terminatePcb(current_process);
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
				//both to avoid a dangling ref, and to help check for a
				//running process.
				current_process = NULL;
			}
		}
		//we have to check again for a current process in case we just started one.
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
