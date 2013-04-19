/*************************************************
//Common fns used by all dispatchers
//Evan Pollack 307278263
*************************************************/
#include "common.h"
#include <signal.h>
#include <unistd.h>


//function to read input file. Returns
//1 if input is expected, otherwise 0.
int
read_input(FILE * file, PcbPtr * head){
	//input handling could be much nicer,
	//but for now we shall just use scanf
	int arrival_time,priority,cputime,memory_alloc,res1,res2,res3,res4;
	Pcb * tmp;
	while(scanf("%d, %d, %d, %d, %d, %d, %d, %d\n", &arrival_time,&priority,&cputime,&memory_alloc,&res1,&res2,&res3,&res4) == 8){
		if(*head == NULL){
			*head = createnullPcb();
			(*head)->arrivaltime = arrival_time;
			(*head)->remainingcputime = cputime;
			(*head)->memory_alloc = memory_alloc;
			(*head)->priority = priority;
			(*head)->args[0] = "./process";
			(*head)->args[1] = NULL;
		}
		else{
			tmp = createnullPcb();
			tmp->arrivaltime = arrival_time;
			tmp->remainingcputime = cputime;
			tmp->memory_alloc = memory_alloc;
			tmp->priority = priority;
			tmp->args[0] = "./process";
			tmp->args[1] = NULL;
			enqPcb(*head, tmp);
		}
	}
	//if we had trouble parsing the file, the pointer will not be at
	//eof- so we print an error then exit
	if(!feof(stdin)){
		return 0;
	}
	return 1;
}

/***************************************
 * functions as described by assign spec
 * *************************************/

PcbPtr
startPcb(PcbPtr process){
	//if the process already exists we must resume it
	if (process->pid){
		if(kill(process->pid,SIGCONT)){
			fprintf(stderr,"unable to continue process %d",process->pid);
		}
		return process;
	}
	//otherwise we must fork a new process
	switch(process->pid = fork()){
		case -1:
			error();
			break;
		case 0:		// child
			execvp(process->args[0],process->args);
			puts("failed to start process");
		default:	// parent
			return process;

	}
	
	return NULL;
}

PcbPtr
terminatePcb(PcbPtr process){
	if(kill(process->pid,SIGINT)){
		fprintf(stderr,"terminate of %d failed",
		(int)process->pid);
		return NULL;
	}
	return process;
}

PcbPtr
createnullPcb(){
	return malloc(sizeof(Pcb));
}

// enqueue
//insertion is O(n). Optimisation would
//be to keep track of the last element
//in head, but this deviates from the 
//assignment spec.
PcbPtr
enqPcb(PcbPtr headofQ, PcbPtr process){
	PcbPtr this = headofQ;
	while(this->next) this = this->next;
	this->next = process;
	return headofQ;
}

// dequeue
PcbPtr
deqPcb(PcbPtr * headofQ){
	PcbPtr dequeued_Pcb = *headofQ;
	//set the head to whatever was next in line
	*headofQ = (*headofQ)->next;
	return dequeued_Pcb;
}

MabPtr memChk(MabPtr m, int size)   // check if memory available
{
	//if the pointer is null, then there is no memory!
	if(!m){
		return -1;
	}
	MabPtr start = m;
	do{
		if(!(m->allocated) && m->size >= size)
			return m;
		m = m->next;
	}
	while(m!=start);
	//if code reaches here, we were unable to find a sufficient
	//unallocated memory block
	return NULL;
}

MabPtr memAlloc(MabPtr m, int size){ // allocate memory block
	//print_blocksizes(m);
	m->next = memSplit(m,size);
	m->size = size;
	m->allocated = 1;
	m->next->next->prev = m->next;
	//print_blocksizes(m);
	return m;
}

MabPtr memFree(MabPtr m){            // free memory block
	m->allocated = 0;
	//if there is only one block of memory we should reset its offset
	//to 0
	if(m == m->next){
		m->offset = 0;
		return m;
	}
	//merge with the next or previous if they are empty, as long
	//as we aren't trying to merge with the same block
	if(!(m->next->allocated)){
		memMerge(m);
	}
	if(!(m->prev->allocated) && m!=m->prev){
		m = memMerge(m->prev);
	}
	//a futile attempt to fix my errors
	if(m->offset + m->size == MAXMEM){
		m->next = m;
	}
	//print_blocksizes(m);
	return m;
}

MabPtr memMerge(MabPtr m){          // merge two memory blocks
	//we need to join the merged item to the next item, both ways
	m->next->next->prev = m;
	m->size = m->size + m->next->size;
	MabPtr temp_next = m->next;
	m->next = m->next->next;
	//make sure we dont free our only block
	if(m != temp_next)
		free(temp_next);
	return m;
}
	
MabPtr memSplit(MabPtr m, int size){ // split a memory block
	MabPtr new_mab = malloc(sizeof(Mab));
	new_mab->size = m->size - size;
	new_mab->prev = m;
	new_mab->next = m->next;
	new_mab->allocated = 0;
	//the new offset will be the sum of the offset and the size
	//of the just allocated block
	new_mab->offset = m->offset + size;
	return new_mab;
}



/**************************************
 * end of assign spec functions
 * ***********************************/


void
error(){
	puts("error");
	exit(EXIT_FAILURE);
}

void
print_arrivaltimes(PcbPtr head){
	while(head){
		printf("AT:     ==%d==\n",head->arrivaltime);
		head = head->next;
	}
}

void
print_blocksizes(MabPtr m){
	MabPtr start = m;
	do{
		printf("=====block of size: %d at offset: %d with next at: %d=====\n",m->size,m->offset,m->next->offset);
		m = m->next;
	}
	while(m!=start);
}

