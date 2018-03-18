/** @file libscheduler.h
 */

#ifndef LIBSCHEDULER_H_
#define LIBSCHEDULER_H_

/**
  Constants which represent the different scheduling algorithms
*/
typedef enum {FCFS = 0, SJF, PSJF, PRI, PPRI, RR} scheme_t;

void  scheduler_start_up               (int cores, scheme_t scheme);
int   scheduler_new_job                (int job_number, int time, int running_time, int priority);
int   scheduler_job_finished           (int core_id, int job_number, int time);
int   scheduler_quantum_expired        (int core_id, int time);
float scheduler_average_turnaround_time();
float scheduler_average_waiting_time   ();
float scheduler_average_response_time  ();
void  scheduler_clean_up               ();

void  scheduler_show_queue             ();

int compare_fcfs(const void* a, const void* b);//-1 means a has higher priority, 1 means b has a higher priority
int compare_sjf(const void* a, const void* b);
int compare_psjf(const void* a, const void* b);
int compare_pri(const void* a, const void* b);
int compare_ppri(const void* a, const void* b);
int compare_rr(const void* a, const void* b);

#endif /* LIBSCHEDULER_H_ */
