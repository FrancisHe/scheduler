/*************************************************
//A round robin dispatcher
//Evan Pollack 307278263
*************************************************/
#include "common.h"
#include "rr.h"
#include <signal.h>
Pcb * input_head = NULL;
Pcb * rr_head = NULL;


int
main(int argc, char** argv){
	if(argc < 1)
		error();
	freopen(argv[1],"r",stdin);
	read_input(stdin,&input_head);
	rr_run();
	return EXIT_SUCCESS;
}

//funtion to run the algorithm
void
rr_run(){
	PcbPtr current_process = NULL;
	int dispatch_timer = 0;
	while(input_head || rr_head || current_process){
		//puts("input q:");
		//print_arrivaltimes(input_head);
		//puts("rr q:");
		//print_arrivaltimes(rr_head);
		while(input_head && input_head->arrivaltime <= dispatch_timer){
			//dequeue the head of the input queue, then add
			//it to the end of the round robin queue.
			//We need to initialize the head if we haven't already
			if(!rr_head){
				rr_head = deqPcb(&input_head);
				rr_head->next = NULL;
			}
			else{
				PcbPtr tmp = deqPcb(&input_head);
				tmp->next = NULL;
				enqPcb(rr_head,tmp);
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
			else if(rr_head){
				if(kill(current_process->pid,SIGTSTP)){
					fprintf(stderr,"temp stop of %d failed",
							(int)current_process->pid);
				}
				enqPcb(rr_head,current_process);
				current_process = NULL;
			}
		}
		if(!current_process && rr_head){
			waitpid(-1, NULL, WUNTRACED);
			current_process = deqPcb(&rr_head);
			current_process->next = NULL;
			startPcb(current_process);
		}
		++dispatch_timer;
		sleep(1);
	}

}
