#include "timeControl.h"
//#include <time.h>
#include <stdio.h>

#include <linux/hrtimer.h>
#include <linux/sched.h>

struct timespec start;
struct timespec now;


/****************************************************************************/
/* Timer variables block                                                    */
/****************************************************************************/
/*static enum hrtimer_restart function_timer(struct hrtimer *);
static struct hrtimer htimer;
static ktime_t kt_periode;*/


void initTime() {
    //does nothing so far
}

void startTime() {
    if(PRINT_RESOLUTION){
        clock_getres(CLOCK_MONOTONIC_RAW, &start);
        printf("The clock's resolution (precision) is %lu second(s) and %lu nanoseconds\n", start.tv_sec, start.tv_nsec);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &start); //TODO check if return val is dif from -1 (error values and stuff)
    clock_gettime(CLOCK_MONOTONIC_RAW, &now); //might be removed
}

double getTimeSinceStart() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return (now.tv_sec - start.tv_sec) + (long double)(now.tv_nsec - start.tv_nsec)/1000000000;
}






/*void initHTimer() {
    //initialization
    kt_periode = ktime_set(0, 100); //seconds,nanoseconds
    hrtimer_init (& htimer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    htimer.function = function_timer;
    hrtimer_start(& htimer, kt_periode, HRTIMER_MODE_REL);
}

void stopHTimer() {
    hrtimer_cancel(& htimer);
}



static enum hrtimer_restart function_timer(struct hrtimer * unused){
        if (gpio_current_state==0){
            gpio_set_value(GPIO_OUTPUT,1);
            gpio_current_state=1;
        }
        else{
            gpio_set_value(GPIO_OUTPUT,0);
            gpio_current_state=0;
        }
        hrtimer_forward_now(& htimer, kt_periode);
        return HRTIMER_RESTART;
}*/
