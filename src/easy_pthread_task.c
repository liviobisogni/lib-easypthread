//*****************************************************************************
//*************** EASY_PTHREAD_TASK.C - For easy task management **************
//***************             Author: Livio Bisogni              **************
//*****************************************************************************

/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
                                  INSTRUCTIONS

    Please read the attached `README.md` file.
_____________________________________________________________________________*/


#include "easy_pthread_task.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    GLOBAL VARIABLES (definition)
_____________________________________________________________________________*/
pthread_t       tid[NT];
struct task_par tp[NT];
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_CREATE:    Create a task with a given period (per, expressed in [ms]),
                    relative deadline (dl_r; expressed in [ms]) and priority
                    (pri; value in {1, ..., LINUX_MAX_PRIO}).
                    The task index (j) is decided by the user.
                    The user must guarantee that:
                        * each task is assigned a different index
                        * the index does not exceed the size of the tp array
                          (i.e., in {0, ..., NT - 1})
_____________________________________________________________________________*/

int task_create(void *(*task)(void *), int j, int per, int dl_r, int pri)
{
    pthread_attr_t     myatt;
    struct sched_param mypar;
    int                tret;

    assert(j < NT);
    assert(j >= 0);
    assert(per >= 0);
    assert(dl_r >= 0);
    assert(pri >= 0);
    assert(pri <= LINUX_MAX_PRIO);

    tp[j].ind    = j;
    tp[j].per    = per;
    tp[j].dl_r   = dl_r;
    tp[j].pri    = pri;
    tp[j].dm     = 0;
    tp[j].rt_avg = 0;
    tp[j].rt_max = 0;
    tp[j].rt_min = DBL_MAX;  // max. finite representable floating-point number
    tp[j].rt_tot = 0;
    tp[j].rt_std = 0;
    tp[j].util_inst = 0;
    tp[j].util_avg  = 0;
    tp[j].ex_tot    = 0;

    tp[j].rt_values = (double *)malloc(GOOGOL * sizeof(double));
    if (!tp[j].rt_values && (GOOGOL * sizeof(double) > 0)) {
        perror("malloc failed!");
        exit(EXIT_FAILURE);
    }

    tp[j].rt_indexes = (unsigned long *)malloc(GOOGOL * sizeof(unsigned long));
    if (!tp[j].rt_indexes && (GOOGOL * sizeof(unsigned long) > 0)) {
        perror("malloc failed!");
        exit(EXIT_FAILURE);
    }

    pthread_attr_init(&myatt);
    pthread_attr_setinheritsched(&myatt, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&myatt, SCHED_RR);
    mypar.sched_priority = tp[j].pri;
    pthread_attr_setschedparam(&myatt, &mypar);

    tret = pthread_create(&tid[j], &myatt, task, (void *)(&tp[j]));

    return tret;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_INDEX: Retrieve task index (stored in tp->ind)
_____________________________________________________________________________*/

int task_get_index(void *ind)
{
    struct task_par *tpar; /* Since the thread argument is passed as a pointer
                              to void, such a pointer must first be converted
                              to a pointer to task_par */

    tpar = (struct task_par *)ind;

    return tpar->ind;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_PERIOD:    Get j-th task period
_____________________________________________________________________________*/

int task_get_period(int j) { return tp[j].per; }
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_MAX_PRIORITY:  Return the priority of the task(s) with the highest
                            one, among the entire task set
_____________________________________________________________________________*/

int task_get_max_priority()
{
    int j;             // task index
    int max_priority;  // maximum task priority

    max_priority = tp[0].pri;

    for (j = 1; j < NT; j++)
        max_priority = fmax(tp[j].pri, max_priority);

    return max_priority;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_MAX_PERIOD:    Return the period of the task(s) with the highest
                            one, among the entire task set
_____________________________________________________________________________*/

int task_get_max_period()
{
    int j;           // task index
    int max_period;  // maximum task period

    max_period = tp[0].per;

    for (j = 1; j < NT; j++)
        max_period = fmax(tp[j].per, max_period);

    return max_period;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_ACTIVATION:    Read the current time, then compute the next
                            activation time and the absolute deadline of the
                            j-th task
_____________________________________________________________________________*/

void task_set_activation(int j)
{
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);
    time_copy(&(tp[j].at), t);
    time_copy(&(tp[j].dl_a), t);
    time_add_ms(&(tp[j].at), tp[j].per);
    time_add_ms(&(tp[j].dl_a), tp[j].dl_r);
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_CHECK_DEADLINE_MISS:   Check whether a deadline miss occured.
                                If yes, increment the amount of deadline misses
                                for the j-th thread and return 1
                                If not, return 0
_____________________________________________________________________________*/

int task_check_deadline_miss(int j)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    if (time_cmp(now, tp[j].dl_a) > 0) {
        tp[j].dm++;
        return 1;
    }

    return 0;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_DEADLINE_MISS: Retrieve the current amount of deadline misses for
                            the j-th task
_____________________________________________________________________________*/

int task_get_deadline_miss(int j) { return tp[j].dm; }
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_DEADLINE_MISS: Set the current amount of deadline misses for the
                            j-th task to dm
_____________________________________________________________________________*/

int task_set_deadline_miss(int j, int dm)
{
    assert(dm >= 0);
    tp[j].dm = dm;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_WAIT_FOR_PERIOD:   Suspend the calling thread until the next
                            activation and, when awaken, update activation time
                            and absolute deadline
_____________________________________________________________________________*/

void task_wait_for_period(int j)
{
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(tp[j].at), NULL);
    time_add_ms(&(tp[j].at), tp[j].per);
    time_add_ms(&(tp[j].dl_a), tp[j].per);
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_WAIT_FOR_END:  Join the j-th task and free the response time arrays
_____________________________________________________________________________*/

int task_wait_for_end(int j)
{
    int rc;  // pthread_join return code (0 in case of success)

    assert(j < NT);
    assert(j >= 0);

    rc = pthread_join(tid[j], NULL);

    free(tp[j].rt_values);
    free(tp[j].rt_indexes);

    return rc;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_RT_AVG:    Get j-th task average response time
_____________________________________________________________________________*/

double task_get_rt_avg(int j) { return tp[j].rt_avg; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_RT_MAX:    Get j-th task maximum response time
_____________________________________________________________________________*/

double task_get_rt_max(int j) { return tp[j].rt_max; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_RT_MIN:    Get j-th task minimum response time
_____________________________________________________________________________*/

double task_get_rt_min(int j) { return tp[j].rt_min; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_RT_TOT:    Get j-th task summation of response times
_____________________________________________________________________________*/

double task_get_rt_tot(int j) { return tp[j].rt_tot; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_RT_STD:    Get j-th task standard deviation of response times
_____________________________________________________________________________*/

double task_get_rt_std(int j) { return tp[j].rt_std; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_UTIL_INST: Get j-th task instantaneous utilization factor
_____________________________________________________________________________*/

double task_get_util_inst(int j) { return tp[j].util_inst; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_UTIL_INST_MAX: Get j-th task maximum instantaneous utilization
                            factor
_____________________________________________________________________________*/

double task_get_util_inst_max(int j) { return tp[j].util_inst_max; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_UTIL_AVG:  Get j-th task average utilization factor
_____________________________________________________________________________*/

double task_get_util_avg(int j) { return tp[j].util_avg; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_RT_VALUES: Get the response time values array of the j-th task
_____________________________________________________________________________*/

double *task_get_rt_values(int j) { return tp[j].rt_values; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_RT_INDEXES:    Get the response time indexes array of the j-th task
_____________________________________________________________________________*/

unsigned long *task_get_rt_indexes(int j) { return tp[j].rt_indexes; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_RT_VALUE:  Get the m-th response time value of the j-th task
_____________________________________________________________________________*/

double task_get_rt_value(int j, unsigned long m) { return tp[j].rt_values[m]; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_RT_INDEX:  Get the m-th response time index of the j-th task
_____________________________________________________________________________*/

unsigned long task_get_rt_index(int j, unsigned long m)
{
    return tp[j].rt_indexes[m];
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_GET_EX_TOT:    Get the (total) number of executions for the j-th task
_____________________________________________________________________________*/

unsigned long task_get_ex_tot(int j) { return tp[j].ex_tot; };
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_RT_AVG:    Set j-th task average response time to rt_avg
_____________________________________________________________________________*/

void task_set_rt_avg(int j, double rt_avg)
{
    assert(rt_avg >= 0);
    tp[j].rt_avg = rt_avg;
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_RT_MAX:    Set j-th task maximum response time to rt_max
_____________________________________________________________________________*/

void task_set_rt_max(int j, double rt_max)
{
    assert(rt_max >= 0);
    tp[j].rt_max = rt_max;
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_RT_MIN:    Set j-th task minimum response time to rt_min
_____________________________________________________________________________*/

void task_set_rt_min(int j, double rt_min)
{
    assert(rt_min >= 0);
    tp[j].rt_min = rt_min;
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_RT_TOT:    Set j-th task summation of response times to rt_tot
_____________________________________________________________________________*/

void task_set_rt_tot(int j, double rt_tot)
{
    assert(rt_tot >= 0);
    tp[j].rt_tot = rt_tot;
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_INCREMENT_RT_TOT:  Increment j-th task summation of response times by
                            rt_value
                            Note: a negative value decreases it by |rt_value|
_____________________________________________________________________________*/

void task_increment_rt_tot(int j, double rt_value)
{
    tp[j].rt_tot += rt_value;
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_RT_STD:    Set j-th task standard deviation of response times to
                        rt_std
_____________________________________________________________________________*/

void task_set_rt_std(int j, double rt_std)
{
    assert(rt_std >= 0);
    tp[j].rt_std = rt_std;
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_UTIL_INST: Set j-th task instantaneous utilization factor to
                        util_inst
_____________________________________________________________________________*/

void task_set_util_inst(int j, double util_inst)
{
    assert(util_inst >= 0);
    tp[j].util_inst = util_inst;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_UTIL_INST_MAX: Set j-th task maximum instantaneous utilization
                            factor to util_inst_max
_____________________________________________________________________________*/

void task_set_util_inst_max(int j, double util_inst_max)
{
    assert(util_inst_max >= 0);
    tp[j].util_inst_max = util_inst_max;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_UTIL_AVG:  Set j-th task average utilization factor to util_avg
_____________________________________________________________________________*/

void task_set_util_avg(int j, double util_avg)
{
    assert(util_avg >= 0);
    tp[j].util_avg = util_avg;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_RT_VALUES: Set the response time values array (of length dim) for
                        the j-th task to rt_values
_____________________________________________________________________________*/

void task_set_rt_values(int j, double rt_values[], unsigned long dim)
{
    unsigned long l;  // for-loop index

    assert(dim > 0);
    assert(dim <= GOOGOL);
    for (l = 0; l < dim; l++)
        tp[j].rt_values[l] = rt_values[l];
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_RT_INDEXES:    Set the response time indexes array (of length dim)
                            for the j-th task to rt_indexes
_____________________________________________________________________________*/

void task_set_rt_indexes(int j, unsigned long rt_indexes[], unsigned long dim)
{
    unsigned long l;  // for-loop index

    assert(dim > 0);
    assert(dim <= GOOGOL);
    for (l = 0; l < dim; l++)
        tp[j].rt_indexes[l] = rt_indexes[l];
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_RT_VALUE:  Set the m-th response time value for the j-th task
                        to rt_value
_____________________________________________________________________________*/

void task_set_rt_value(int j, unsigned long m, double rt_value)
{
    assert(m >= 0);
    assert(m < GOOGOL);
    assert(rt_value >= 0);
    tp[j].rt_values[m] = rt_value;
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_RT_INDEX:  Set the m-th response time index for the j-th task
                        to rt_index
_____________________________________________________________________________*/

void task_set_rt_index(int j, unsigned long m, unsigned long rt_index)
{
    assert(m >= 0);
    assert(m < GOOGOL);
    assert(rt_index >= 0);
    tp[j].rt_indexes[m] = rt_index;
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_SET_EX_TOT:    Set the (total) number of executions for the j-th task
                        to m
_____________________________________________________________________________*/

void task_set_ex_tot(int j, unsigned long m)
{
    assert(m >= 0);
    assert(m < GOOGOL);
    tp[j].ex_tot = m;
};
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_RT_MAX:    Compute maximum response time of the j-th task
_____________________________________________________________________________*/

double task_compute_rt_max(int j)
{
    unsigned long m;
    double        rt_value;  // last response time value
    double        rt_max;

    m        = tp[j].ex_tot;
    rt_value = tp[j].rt_values[m];
    rt_max   = tp[j].rt_max;

    rt_max = fmax(rt_value, rt_max);
    // return fmax(tp[j].rt_values[tp[j].ex_tot], tp[j].rt_max); // shorter way

    return rt_max;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_RT_MAX_FROM_SCRATCH:   Compute maximum response time of the
                                        j-th task from scratch
_____________________________________________________________________________*/

double task_compute_rt_max_from_scratch(int j)
{
    unsigned long m;
    unsigned long ex_tot;
    double       *rt_values;
    double        rt_max;

    ex_tot    = tp[j].ex_tot;
    rt_values = tp[j].rt_values;
    rt_max    = 0;

    for (m = 0; m < ex_tot; m++)
        rt_max = fmax(rt_values[m], rt_max);

    return rt_max;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_RT_MIN:    Compute minimum response time of the j-th task
_____________________________________________________________________________*/

double task_compute_rt_min(int j)
{
    unsigned long m;
    double        rt_value;  // last response time value
    double        rt_min;

    m        = tp[j].ex_tot;
    rt_value = tp[j].rt_values[m];
    rt_min   = tp[j].rt_min;

    rt_min = fmin(rt_value, rt_min);
    // return fmin(tp[j].rt_values[tp[j].ex_tot], tp[j].rt_min); // shorter way

    return rt_min;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_RT_MIN_FROM_SCRATCH:   Compute minimum response time of the
                                        j-th task from scratch
_____________________________________________________________________________*/

double task_compute_rt_min_from_scratch(int j)
{
    unsigned long m;
    unsigned long ex_tot;
    double       *rt_values;
    double        rt_min;

    ex_tot    = tp[j].ex_tot;
    rt_values = tp[j].rt_values;
    rt_min    = DBL_MAX;

    for (m = 0; m < ex_tot; m++)
        rt_min = fmin(rt_values[m], rt_min);

    return rt_min;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_STD_DEV:   Compute standard deviation of the response time of
                            the j-th task
_____________________________________________________________________________*/

double task_compute_std_dev(int j)
{
    double       *rt_values;
    unsigned long m;
    double        rt_avg;
    unsigned long l;            // for-loop index
    double        squares_sum;  // summation of (x - x_avg)^2
    double        rt_std;       // standard deviation

    rt_values   = tp[j].rt_values;
    m           = tp[j].ex_tot;
    rt_avg      = tp[j].rt_avg;
    squares_sum = 0;

    if (m == 0)  // When there is only one sample:
        return DBL_MAX;
    for (l = 0; l < m; l++)
        squares_sum += pow((rt_values[l] - rt_avg), 2);

    /* Estimate the standard deviation using the corrected sample standard
       deviation (dividing by N_samples - 1 = m rather than by N_samples = m
       + 1, i.e., using the Bessel's correction) */
    rt_std = sqrt(squares_sum / m);

    return rt_std;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_UTIL_INST: Compute instantaneous utilization factor of the
                            j-th task
_____________________________________________________________________________*/

double task_compute_util_inst(int j)
{
    double        rt_current;  // current response time
    unsigned long m;           // number of task executions
    int           per;         // task period
    double        util_inst;   /* instantaneous utilization factor; value
                                  (hopefully) in [0, 1] */

    m          = tp[j].ex_tot;
    rt_current = tp[j].rt_values[m];
    per        = tp[j].per;

    util_inst = rt_current / (double)(per);

    return util_inst;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_UTIL_INST_MAX: Compute maximum instantaneous utilization
                                factor of the j-th task
_____________________________________________________________________________*/

double task_compute_util_inst_max(int j)
{
    double rt_max;        // maximum response time
    int    per;           // task period
    double util_inst_max; /* maximum instantaneous utilization factor;
                             value (hopefully) in [0, 1] */
    double util_inst;     /* instantaneous utilization factor; value
                             (hopefully) in [0, 1] */

    rt_max = tp[j].rt_max;
    per    = tp[j].per;

    util_inst_max = rt_max / (double)(per);
    // util_inst_max = fmax(util_inst, util_inst_max);   // alternative way

    return util_inst_max;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_UTIL_AVG:  Compute average utilization factor of the j-th task
_____________________________________________________________________________*/

double task_compute_util_avg(int j)
{
    double        rt_tot;
    unsigned long m;
    int           per;
    double        util_avg; /* average utilization factor; value
                               (hopefully) in [0, 1] */

    rt_tot = tp[j].rt_tot;
    m      = tp[j].ex_tot;
    per    = tp[j].per;

    util_avg = rt_tot / (double)((m + 1) * per);

    return util_avg;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_RT_AVG:    Compute average response time of the j-th task
_____________________________________________________________________________*/

double task_compute_rt_avg(int j)
{
    double        rt_tot;
    double        rt_avg;  // average response time
    unsigned long m;

    rt_tot = tp[j].rt_tot;
    m      = tp[j].ex_tot;

    rt_avg = rt_tot / (m + 1);

    return rt_avg;
}
//-----------------------------------------------------------------------------


/*‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    TASK_COMPUTE_RT_AVG_FROM_SCRATCH:   Compute average response time of the
                                        j-th task from scratch
_____________________________________________________________________________*/

double task_compute_rt_avg_from_scratch(int j)
{
    unsigned long m;
    unsigned long ex_tot;
    double       *rt_values;
    double        rt_tot;
    double        rt_avg;  // average response time

    ex_tot    = tp[j].ex_tot;
    rt_values = tp[j].rt_values;
    rt_tot    = 0;

    for (m = 0; m <= ex_tot; m++)
        rt_tot += tp[j].rt_values[m];

    rt_avg = rt_tot / (ex_tot + 1);

    return rt_avg;
}
//-----------------------------------------------------------------------------
