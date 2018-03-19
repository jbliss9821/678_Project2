/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.
  You may need to define some global variables or a struct to store your job queue elements.
*/
typedef struct _core
{
	int busy;//1 busy, 0 not busy
	job_t* current_job;

}core;

typedef struct _job_t
{
	int job_id;
	int arrival_time;
	int running_time;
	int remaining_time;
	int waiting_time;
	int turnaround_time;
	int priority;

} job_t;

typedef struct _scheduler_instance
{
	int num_jobs;
	int num_cores;
	float total_wait;
	float total_turnaround;
	float total_response;
	int previous_time;
	scheme_t scheme;
	priqueue_t queue;
	core* core_array;

} scheduler_instance;


scheduler_instance schedule;

/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.
  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme_in)
{
	schedule.total_response = 0;
	schedule.total_turnaround = 0;
	schedule.total_wait = 0;
	schedule.previous_time = 0;
	schedule.num_jobs = 0;
	schedule.num_cores = cores;
	schedule.scheme = scheme_in;
	schedule.core_array = malloc(sizeof(core) * schedule.num_cores);
	for (int i = 0; i < schedule.num_cores; i++)
	{
		schedule.core_array[i].busy = 0;
		schedule.core_array[i].current_job = NULL;
	}

	if (scheme_in == FCFS)
	{
		priqueue_init(&(schedule.queue), compare_fcfs);
	}
	else if(scheme_in == SJF)
	{
		priqueue_init(&(schedule.queue), compare_sjf);
	}
	else if(scheme_in == PSJF)
	{
		priqueue_init(&(schedule.queue), compare_psjf);
	}
	else if(scheme_in == PRI)
	{
		priqueue_init(&(schedule.queue), compare_pri);
	}
	else if(scheme_in == PPRI)
	{
		priqueue_init(&(schedule.queue), compare_ppri);
	}
	else if(scheme_in == RR)
	{
		priqueue_init(&(schedule.queue), compare_rr);
	}
	else//default to FCFS
	{
		priqueue_init(&(schedule.queue), compare_fcfs);
	}
}


/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.
  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
	//keep track of each running time as the new time for the next job comes in
	//need an array jobs that are in to keep track of times
	//keep track of: wait time turnaround time, response time

	//TODO: update times based on time provided here (using a new function)
	update_times(time);

	//new job creation
	job_t* new_job = malloc(sizeof(job_t));
	new_job->job_id = job_number;
	new_job->arrival_time = time;
	new_job->running_time = running_time;
	new_job->waiting_time = 0;
	new_job->turnaround_time = -1;
	new_job->priority = priority;//(The lower the value, the higher the priority.)

	//finding a core
	int core_to_use = -1;//-1 means no core found

	//preemption variables
	int swap = 0;//0 to not swap/preempt, 1 to swap/preempt
	int core_to_swap = -1;//the index of the core that will be preempted
	job_t* job_to_swap = NULL;//pointer to job to be preempted
	int job_to_swap_remaining_time = -1;
	int job_to_swap_priority = -1;

	for (int i = schedule.num_cores-1; i >= 0; i--)//work towards lower index core for core selection
	{
		if (schedule.core_array[i].busy == 0)
		{
			core_to_use = i;
		}
	}

	if(core_to_use != -1)//if there was a non busy core, put that ish in
	{
		new_job->turnaround_time = 0;
		return(core_to_use);
	}
	else//no open core was found, need to check for preemption or add to queue
	{
		//check for preemption
		if (schedule.scheme == PSJF)
		{
			//search cores for a job with a longer remaining_time
			//if a longer remaining time is found, replace it with the current job
			for (int i = schedule.num_cores-1; i >= 0; i--)
			{
				job_to_swap = schedule.core_array[i].current_job;
				if (job_to_swap->remaining_time > new_job->running_time && job_to_swap->remaining_time > job_to_swap_remaining_time)
				{
					//if it is greater than the current job run time
					//and if it is bigger than the current job to swap running time(it should be the job with the most remaining time)
					core_to_swap = i;
					job_to_swap_remaining_time = job_to_swap->remaining_time;
					swap = 1;//can swap now since a core that can be preempted has been found
				}
			}
		}
		else if(schedule.scheme == PPRI)
		{
			//search for a job with a lower priority
			//if a lower priority job is found, replace with the current job
			for (int i = schedule.num_cores-1; i >= 0; i--)
			{
				job_to_swap = schedule.core_array[i].current_job;
				if (job_to_swap->priority > new_job->priority && job_to_swap->priority > job_to_swap_priority)
				{
					//if job to swap priority is lower (has a larger value)
					//and if job to swap priority is lower(has a larger value) than the current job to swap priority(it should be the job with the lowest priority)
					core_to_swap = i;
					job_to_swap_priority = job_to_swap->priority;
					swap = 1;//can swap now since a core that can be preempted has been found
				}
			}
		}

		if (swap == 1 && core_to_swap != -1)//if can swap, swap
		{
			job_to_swap = schedule.core_array[core_to_swap].current_job;
			schedule.core_array[core_to_swap].current_job = new_job;
			priqueue_offer(&schedule.queue, job_to_swap);
			new_job->turnaround_time = 0;//turnaround is 0 since the job is getting scheduled now
			return(core_to_swap);
		}
		else//if no swap available for preemption or not a preemption scheme, queue the new job
		{
			priqueue_offer(&schedule.queue, new_job);
			return (-1);//no core to use
		}
	}

	return -1;
}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
	job_t* current_job = Priqueue_poll(&schedule.queue);

	//TODO: when a job finishes, add the times to the total times for the schedule
	schedule.total_response = schedule.total_response + current_job->response_time;
	schedule.total_wait = schedule.total_wait + current_job->waiting_time;
	schedule.total_turnaround = schedule.total_turnaround + current_job->turnaround_time;

	//TODO: when a job finishes, adjust the response time
	current_job->response_time = time - current_job->arrival_time

	priqueue.remove(&schedule.queue, current_job);
	Free(current_job);
	current_job = NULL;

	job_t* next_job = Priqueue_poll(&schedule.queue);
	if (next_job == NULL)
	{
		return -1;
	}
	else
	{
		//TODO: when putting in a new job from the queue, adjust the turnaround time if it hasn't been scheduled yet
		//if next_job->turnaround_time == -1, next_job->turnaround_time = time - next_job->arrival_time;
		if (next_job -> turnaround_time == -1)
		{
			next_job -> turnaround_time = time - next_job -> arrival_time;
		}
		return next_job -> job_id;
	}
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.
  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
	return -1;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.
  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	float avg_wait = 0.0;

	if (schedule.num_jobs == 0)
	{
		avg_wait = 0.0;
	}
	else
	{
		avg_wait = schedule.total_wait / schedule.num_jobs;
	}

	return (avg_wait);
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.
  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	float avg_turnaround = 0.0;

	if (schedule.num_jobs == 0)
	{
		avg_turnaround = 0.0;
	}
	else
	{
		avg_turnaround = schedule.total_turnaround / schedule.num_jobs;
	}

	return (avg_turnaround);
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.
  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	float avg_response = 0.0;

	if (schedule.num_jobs == 0)
	{
		avg_response = 0.0;
	}
	else
	{
		avg_response = schedule.total_response / schedule.num_jobs;
	}

	return (avg_response);
}


/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{

}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:
    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{

}

/**
Compare Functions
*/

int compare_fcfs(const void* a, const void* b)//-1 means a has higher priority, 1 means b has a higher priority
{
	job_t job_a = *((job_t*)a);
	job_t job_b = *((job_t*)b);
	if (job_a.arrival_time < job_b.arrival_time)
	{
		return(-1);
	}
	else if (job_a.arrival_time > job_b.arrival_time)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

int compare_sjf(const void* a, const void* b)
{
	job_t job_a = *((job_t*)a);
	job_t job_b = *((job_t*)b);
	if(job_a.running_time < job_b.running_time)
	{
		return(-1);
	}
	else if (job_a.running_time > job_b.running_time)
	{
		return(1);
	}
	else
	{
		if (job_a.arrival_time < job_b.arrival_time)
		{
			return(-1);
		}
		else if (job_a.arrival_time > job_b.arrival_time)
		{
			return(1);
		}
		else
		{
			return (0);
		}
	}
}

int compare_psjf(const void* a, const void* b)
{
	job_t job_a = *((job_t*)a);
	job_t job_b = *((job_t*)b);
	if(job_a.remaining_time < job_b.remaining_time)
	{
		return(-1);
	}
	else if (job_a.remaining_time > job_b.remaining_time)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

int compare_pri(const void* a, const void* b)
{
	job_t job_a = *((job_t*)a);
	job_t job_b = *((job_t*)b);

	if(job_a.priority < job_b.priority)
	{
		return(-1);
	}
	else if (job_a.priority > job_b.priority)
	{
		return(1);
	}
	else
	{
		if (job_a.arrival_time < job_b.arrival_time)
		{
			return(-1);
		}
		else if (job_a.arrival_time > job_b.arrival_time)
		{
			return(1);
		}
		else
		{
			return (0);
		}
	}
}

int compare_ppri(const void* a, const void* b)
{
	job_t job_a = *((job_t*)a);
	job_t job_b = *((job_t*)b);

	if(job_a.priority < job_b.priority)
	{
		return(-1);
	}
	else if (job_a.priority > job_b.priority)
	{
		return(1);
	}
	else
	{
		if (job_a.arrival_time < job_b.arrival_time)
		{
			return(-1);
		}
		else if (job_a.arrival_time > job_b.arrival_time)
		{
			return(1);
		}
		else
		{
			return (0);
		}
	}
}

int compare_rr(const void* a, const void* b)
{
	job_t job_a = *((job_t*)a);
	job_t job_b = *((job_t*)b);

	if (job_a.arrival_time < job_b.arrival_time)
	{
		return(-1);
	}
	else if (job_a.arrival_time > job_b.arrival_time)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

void update_times(int time_in)
{
	int time_dif = time_in - schedule.previous_time;
	schedule.previous_time = time_in;

	for (int i = 0; i < priqueue_size(&schedule.queue); i++)
	{
		job_t* temp_job = (job_t*)(priqueue_at(&schedule.queue, i));
		if (temp_job != NULL)
		{
			temp_job->waiting_time = temp_job->waiting_time + time_diff;
		}
	}

}

























//
