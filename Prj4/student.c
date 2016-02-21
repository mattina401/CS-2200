/*
 * student.c
 * Multithreaded OS Simulation for CS 2200, Project 4
 * Spring 2015
 *
 * This file contains the CPU scheduler for the simulation.
 * Name: Jongyeon Kim
 * GTID: 903018469
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "os-sim.h"

#include <string.h>


/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 */
static pcb_t **current;
static pthread_mutex_t current_mutex;

static pthread_mutex_t queue_mutex;
static pthread_cond_t queue_cond;

typedef struct {
	pcb_t *head;
	pcb_t *next;
}linked_list;

static linked_list *ready_queue;
static int timeslice;
static int which_scheduler;
static int num_CPU;



static void add_end(pcb_t *p) {
	
	pcb_t* tmp;
	pthread_mutex_lock(&queue_mutex);
	if(ready_queue -> head == NULL) {
		ready_queue -> head = p;
	}
	else {
		tmp=ready_queue -> head;
		while(tmp -> next != NULL) 
			tmp = tmp -> next;
		tmp -> next = p;
	}
	pthread_cond_signal(&queue_cond);
	pthread_mutex_unlock(&queue_mutex);
}

static pcb_t* remove_front() {

    	pcb_t* ret = ready_queue -> head; 
    	pcb_t* ret_prev;
    	pcb_t* tmp; 
	pcb_t* tmp_prev;
    
    	pthread_mutex_lock(&queue_mutex);
    	if(ready_queue -> head == NULL) {
        	pthread_mutex_unlock(&queue_mutex);
        	return NULL;
    	}
    
    	
    	if(which_scheduler == 2) {
        	tmp = ret;
        	while(tmp -> next != NULL) {
            		if(tmp -> static_priority > ret -> static_priority){
                		ret = tmp;
                		ret_prev = tmp_prev;
            		}
            	tmp_prev = tmp;
		tmp = tmp->next;
        }
        
        	if(ret == ready_queue -> head) {
        		ready_queue -> head = ret -> next;
		} else {
        		ret_prev -> next=ret -> next;
		}
    	} else {
    		ret = ready_queue -> head;
    		ready_queue -> head = ret -> next;
    	}
    
    	ret -> next = NULL;
    	pthread_mutex_unlock(&queue_mutex);
    
    	return ret;
}


/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which 
 *	you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING
 *
 *   3. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *	The current array (see above) is how you access the currently running process indexed by the cpu id. 
 *	See above for full description.
 *	context_switch() is prototyped in os-sim.h. Look there for more information 
 *	about it and its parameters.
 */
static void schedule(unsigned int cpu_id)
{

    
    	pcb_t* p=remove_front();
    	pthread_mutex_lock(&current_mutex);
    
    	if(p!=NULL) {
    		p->state=PROCESS_RUNNING;
	}

    	context_switch(cpu_id, p, timeslice);
    	
    	current[cpu_id]=p;
    	pthread_mutex_unlock(&current_mutex);  
}


/*
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * This function should block until a process is added to your ready queue.
 * It should then call schedule() to select the process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
	/*block current mutex*/
    	pthread_mutex_lock(&current_mutex);

	/*until process can be picked up*/
	while(ready_queue -> head == NULL) {
       		pthread_cond_wait(&queue_cond, &current_mutex);
	}
  	pthread_mutex_unlock(&current_mutex);
  	schedule(cpu_id);

    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     */
    /*mt_safe_usleep(1000000);*/
}


/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 */
extern void preempt(unsigned int cpu_id)
{
    	pthread_mutex_lock(&current_mutex);
    
    	current[cpu_id]->state=PROCESS_READY;
    	add_end(current[cpu_id]);
    
    	pthread_mutex_unlock(&current_mutex);
    	schedule(cpu_id);

}


/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    	pthread_mutex_lock(&current_mutex);
    	current[cpu_id] -> state = PROCESS_WAITING;
    	pthread_mutex_unlock(&current_mutex);   
	schedule(cpu_id);
}


/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    	pthread_mutex_lock(&current_mutex); 
    	current[cpu_id] -> state = PROCESS_TERMINATED;
    	pthread_mutex_unlock(&current_mutex);   
    	schedule(cpu_id);
}


/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 *   2. If the scheduling algorithm is static priority, wake_up() may need
 *      to preempt the CPU with the lowest priority process to allow it to
 *      execute the process which just woke up.  However, if any CPU is
 *      currently running idle, or all of the CPUs are running processes
 *      with a higher priority than the one which just woke up, wake_up()
 *      should not preempt any CPUs.
 *	To preempt a process, use force_preempt(). Look in os-sim.h for 
 * 	its prototype and the parameters it takes in.
 */
extern void wake_up(pcb_t *process)
{
    	int i = 0;
    	int p = -1;
    
    	if(which_scheduler == 2) {
        	pthread_mutex_lock(&current_mutex);
        	while((i < num_CPU) && (current[i] != NULL) && (p == -1)) {
            		if(current[i] -> static_priority < process -> static_priority) {
                		p = i;
            		}
            	i++;
        	}
        	if(p != -1) {
            		pthread_mutex_unlock(&current_mutex);
            		force_preempt(p);
            		pthread_mutex_lock(&current_mutex);
        	}
        	pthread_mutex_unlock(&current_mutex);
    	}
    
    	process -> state = PROCESS_READY;
    	add_end(process);
  
}


/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -r and -p command-line parameters.
 */
int main(int argc, char *argv[])
{
	int cpu_count;
	
	if(argc == 4 && !(strcmp(argv[2], "-r"))) {
        	printf("Round-Robin Scheduler\n");
        	which_scheduler = 1;
        	timeslice = atoi(argv[3]);

    	} else if(argc == 3 && !(strcmp(argv[2], "-p"))) {
        	printf("Static Priority Scheduler\n");
        	which_scheduler = 2;
        	timeslice = -1;
    	} else if (argc != 2) {
        	fprintf(stderr, "CS 2200 Project 4 -- Multithreaded OS Simulator\n"
            	"Usage: ./os-sim <# CPUs> [ -r <time slice> | -p ]\n"
            	"    Default : FIFO Scheduler\n"
            	"         -r : Round-Robin Scheduler\n"
            	"         -p : Static Priority Scheduler\n\n");
        	return -1;
    	} else {
        	printf("FCFS Scheduler\n");
        	which_scheduler = 0;
        	timeslice = -1;
    	}

    	cpu_count = atoi(argv[1]);

	num_CPU = cpu_count;

    	current = malloc(sizeof(pcb_t*) * cpu_count);
    	assert(current != NULL);
    
    	ready_queue = (linked_list *)malloc(sizeof(linked_list));
    	assert(ready_queue != NULL);

	ready_queue -> head=NULL;
    
    	pthread_mutex_init(&current_mutex, NULL);
    	pthread_mutex_init(&queue_mutex,NULL);
    	pthread_cond_init(&queue_cond,NULL);
    	start_simulator(cpu_count);

    	return 0;
}

