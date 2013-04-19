/*************************************************
//A first-come-first-serve dispatcher
//Evan Pollack 307278263
*************************************************/
#include "common.h"
#include "fcfs.h"
#include <signal.h>
Pcb * head = NULL;


int
main(int argc, char** argv){
	if(argc < 1)
		error();
	freopen(argv[1],"r",stdin);
	read_input(stdin,&head);
	fcfs_run();
	return EXIT_SUCCESS;
}

//funtion to run the algorithm
void
fcfs_run(){
	PcbPtr current_process = NULL;
	int dispatch_timer = 0;
	while(head || current_process){
		//print_arrivaltimes(head);
		if(current_process){
			current_process->remainingcputime--;
			if(current_process->remainingcputime <= 0){
				terminatePcb(current_process);
				free(current_process);
				current_process = NULL;
				
			}
		}
		if(!current_process && head && head->arrivaltime <= dispatch_timer){
			current_process = deqPcb(&head);
			startPcb(current_process);
		}
		else{

		}
		++dispatch_timer;
		sleep(1);
	}

}
