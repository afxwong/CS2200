/*
 * student.c
 * Multithreaded OS Simulation for CS 2200
 *
 * This file contains the CPU scheduler for the simulation.
 */

#include <assert.h>
#include <pthread.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "student.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);



/**
 * current is an array of pointers to the currently running processes, 
 * each pointer corresponding to each CPU in the simulation.
 */
static pcb_t **current;
/* rq is a pointer to a struct you should use for your ready queue implementation.*/
static queue_t *rq;

/**
 * current and rq are accessed by multiple threads, so you will need to use 
 * a mutex to protect it (ready queue).
 *
 * The condition variable queue_not_empty has been provided for you
 * to use in conditional waits and signals.
 */
static pthread_mutex_t current_mutex;
static pthread_mutex_t queue_mutex;
static pthread_cond_t queue_not_empty;

/* keeps track of the scheduling alorightem and cpu count */
static sched_algorithm_t scheduler_algorithm;
static unsigned int cpu_count;

static int timeslice;

/** ------------------------Problem 0 & 2-----------------------------------
 * Checkout PDF Section 2 and 4 for this problem
 * 
 * enqueue() is a helper function to add a process to the ready queue.
 *
 * @param queue pointer to the ready queue
 * @param process process that we need to put in the ready queue
 */
void enqueue(queue_t *queue, pcb_t *process)
{
    pthread_mutex_lock(&queue_mutex);
    if (is_empty(queue))
    {
        queue->head = process;
        queue->tail = process;
    }
    else 
    {
        queue->tail->next = process;
        queue->tail = process;
    }
    process->next = NULL;
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_mutex);
}

/**
 * dequeue() is a helper function to remove a process to the ready queue.
 *
 * @param queue pointer to the ready queue
 */
pcb_t *dequeue(queue_t *queue)
{
    pthread_mutex_lock(&queue_mutex);
    pcb_t *out;
    if (is_empty(queue))
    {
        out = NULL;
    }
    else
    {
        if (scheduler_algorithm == PR)
        {
            unsigned int max_pr = __INT_MAX__;
            pcb_t *curr = queue->head;
            while (curr)
            {
                if (curr->priority < max_pr)
                {
                    max_pr = curr->priority;
                    out = curr;
                }
                curr = curr->next;
            }
            if (queue->head == out)
            {
                queue->head = queue->head->next;
            }
            else
            {
                pcb_t *curr2 = queue->head->next;
                pcb_t *prev = queue->head;
                while (curr2)
                {
                    if (curr2 == out)
                    {
                        prev->next = curr2->next;
                        if (curr2 == queue->tail)
                        {
                            queue->tail = prev;
                        }
                    }
                    curr2 = curr2->next;
                    prev = prev->next;
                }
            }
        }
        else
        {
            out = queue->head;
            if (queue->head == queue->tail)
            {
                queue->head = NULL;
                queue->tail = NULL;
            }
            else
            {
                queue->head = queue->head->next;
            }
        }
    }
    pthread_mutex_unlock(&queue_mutex);
    return out;
}

/** ------------------------Problem 0-----------------------------------
 * Checkout PDF Section 2 for this problem
 * 
 * is_empty() is a helper function that returns whether the ready queue
 * has any processes in it.
 * 
 * @param queue pointer to the ready queue
 * 
 * @return a boolean value that indicates whether the queue is empty or not
 */
bool is_empty(queue_t *queue)
{
    return queue->head == NULL;
}

/** ------------------------Problem 1B & 3-----------------------------------
 * Checkout PDF Section 3 and 5 for this problem
 * 
 * schedule() is your CPU scheduler.
 * 
 * Remember to specify the timeslice if the scheduling algorithm is Round-Robin
 * 
 * @param cpu_id the target cpu we decide to put our process in
 */
static void schedule(unsigned int cpu_id)
{
    pcb_t *proc = dequeue(rq);
    if (proc) proc->state = PROCESS_RUNNING;
    pthread_mutex_lock(&current_mutex);
    current[cpu_id] = proc;
    context_switch(cpu_id, proc, timeslice);
    pthread_mutex_unlock(&current_mutex);
}

/**  ------------------------Problem 1A-----------------------------------
 * Checkout PDF Section 3 for this problem
 * 
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * @param cpu_id the cpu that is waiting for process to come in
 */
extern void idle(unsigned int cpu_id)
{
    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     */
    pthread_mutex_lock(&queue_mutex);
    while(is_empty(rq))
    {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    pthread_mutex_unlock(&queue_mutex);
    schedule(cpu_id);
}

/** ------------------------Problem 2 & 3-----------------------------------
 * Checkout Section 4 and 5 for this problem
 * 
 * preempt() is the handler used in Round-robin and Preemptive Priority 
 * Scheduling
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 * 
 * @param cpu_id the cpu in which we want to preempt process
 */
extern void preempt(unsigned int cpu_id)
{
    current[cpu_id]->state = PROCESS_READY;
    enqueue(rq, current[cpu_id]);
    current[cpu_id] = NULL;
    schedule(cpu_id);
}

/**  ------------------------Problem 1-----------------------------------
 * Checkout PDF Section 3 for this problem
 * 
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * @param cpu_id the cpu that is yielded by the process
 */
extern void yield(unsigned int cpu_id)
{
    current[cpu_id]->state = PROCESS_WAITING;
    current[cpu_id] = NULL;
    schedule(cpu_id);
}

/**  ------------------------Problem 1-----------------------------------
 * Checkout PDF Section 3
 * 
 * terminate() is the handler called by the simulator when a process completes.
 * 
 * @param cpu_id the cpu we want to terminate
 */
extern void terminate(unsigned int cpu_id)
{
    current[cpu_id]->state = PROCESS_TERMINATED;
    current[cpu_id] = NULL;
    schedule(cpu_id);
}

/**  ------------------------Problem 1A & 3---------------------------------
 * Checkout PDF Section 3 and 4 for this problem
 * 
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes. This method will also need to handle priority, 
 * Look in section 5 of the PDF for more info.
 * 
 * @param process the process that finishes I/O and is ready to run on CPU
 */
extern void wake_up(pcb_t *process)
{
    process->state = PROCESS_READY;
    enqueue(rq, process);
    bool alllower = true;
    bool hasidle = false;
    if (scheduler_algorithm == PR)
    {
        pthread_mutex_lock(&current_mutex);
        // long unsigned int elements = sizeof(pcb_t) / sizeof(current[0]);
        for (long unsigned int i = 0; i < cpu_count; i++)
        {
            if (!current[i])
            {
                hasidle = true;
                break;
            }
        }
        pthread_mutex_unlock(&current_mutex);
        if (hasidle) return;
        pthread_mutex_lock(&current_mutex);
        for (long unsigned int i = 0; i < cpu_count; i++)
        {
            if (process->priority < current[i]->priority)
            {
                alllower = false;
                break;
            }
        }
        pthread_mutex_unlock(&current_mutex);
        if (alllower) return;
        pthread_mutex_lock(&current_mutex);
        unsigned int highest = 0;
        unsigned int idx = __INT_MAX__;
        for (unsigned int i = 0; i < cpu_count; i++)
        {
            if (current[i]->priority > highest)
            {
                idx = i;
                highest = current[i]->priority;
            }
        }
        pthread_mutex_unlock(&current_mutex);
        force_preempt(idx);
    }
}

/**
 * main() simply parses command line arguments, then calls start_simulator().
 * Add support for -r and -p parameters. If no argument has been supplied, 
 * you should default to FCFS.
 * 
 * HINT:
 * Use the scheduler_algorithm variable (see student.h) in your scheduler to 
 * keep track of the scheduling algorithm you're using.
 */
int main(int argc, char *argv[])
{

    int opt;
    scheduler_algorithm = FCFS;

    if (argc < 2)
    {
        fprintf(stderr, "CS 2200 Project 4 -- Multithreaded OS Simulator\n"
                        "Usage: ./os-sim <# CPUs> [ -r <time slice> | -p ]\n"
                        "    Default : FCFS Scheduler\n"
                        "         -r : Round-Robin Scheduler\n1\n"
                        "         -p : Priority Scheduler\n");
        return -1;
    }
    else
    {
        opt = getopt(argc, argv, "r:p");
        switch(opt)
        {
            case 'r':
                timeslice = atoi(optarg);
                scheduler_algorithm = RR;
                break;
            case 'p':
                scheduler_algorithm = PR;
                break;
            default:
                break;
        }
    }


    /* Parse the command line arguments */
    cpu_count = strtoul(argv[1], NULL, 0);

    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t *) * cpu_count);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_not_empty, NULL);
    rq = (queue_t *)malloc(sizeof(queue_t));
    assert(rq != NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);

    return 0;
}

#pragma GCC diagnostic pop
