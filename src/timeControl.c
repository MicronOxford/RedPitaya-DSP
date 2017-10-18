#include "timeControl.h"
#include <time.h>
#include <stdio.h>

struct timespec start;
struct timespec now;

void initTime() {

}//does nothing so far

void startTime() {
    if(PRINT_RESOLUTION){
        clock_getres(CLOCK_MONOTONIC_RAW, &start);
        printf("The clock's resolution (precision) is %lu second(s) and %lu nanoseconds\n", start.tv_sec, start.tv_nsec);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &start); //TODO check if return val is dif from -1 (error values and stuff)
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
}

double getTimeSinceStart() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return (now.tv_sec - start.tv_sec) + (long double)(now.tv_nsec - start.tv_nsec)/1000000000;
}