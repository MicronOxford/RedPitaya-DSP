#include "timeControl.h"
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>


#define BLOCK_SIZE (4*1024)


struct timespec start;
struct timespec now;
struct timespec next;



// clock access
volatile unsigned *ARMTimer;

long unsigned int lsbARMTimer;
long unsigned int msbARMTimer;
long unsigned int lsbNextTime;
long unsigned int msbNextTime;



void startTime() {
    if(PRINT_RESOLUTION){
        clock_getres(CLOCK_MONOTONIC_RAW, &start);
        printf("The clock's resolution (precision) is %lu second(s) and %lu nanoseconds\n", start.tv_sec, start.tv_nsec);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &start); //TODO check if return val is dif from -1 (error values and stuff)
    clock_gettime(CLOCK_MONOTONIC_RAW, &now); //might be removed
    clock_gettime(CLOCK_MONOTONIC_RAW, &next);
}

double getTimeSinceStart() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return (now.tv_sec - start.tv_sec) + (long double)(now.tv_nsec - start.tv_nsec)/1000000000;
}

int compareTimeToNow(struct timespec *time) { //return 0 if now > next
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    if(now.tv_sec != time->tv_sec) {
        return (now.tv_sec < time->tv_sec);
    }
    return now.tv_nsec < time->tv_nsec;
}

int compareNextToNow() { //return 0 if now > next
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    if(now.tv_sec != next.tv_sec) {
        return (now.tv_sec < next.tv_sec);
    }
    return now.tv_nsec < next.tv_nsec;
}

void addNanoSecToTime(long nsec, struct timespec *time) {
    /* TODO
    if(nsec > 2*BILLION) { return ERROR; }
    */
    long nanoseconds = time->tv_nsec + nsec;
    if(nanoseconds > BILLION) {
        nanoseconds -= BILLION;
        time->tv_sec++;
    }
    time->tv_nsec = nanoseconds;
}

void addNanoSecToNext(long nsec) {
    /* TODO
    if(nsec > 2*BILLION) { return ERROR; }
    */
    addNanoSecToTime(nsec, &next);
    // long nanoseconds = next.tv_nsec + nsec;
    // if(nanoseconds > BILLION) {
    //     nanoseconds -= BILLION;
    //     next.tv_sec++;
    // }
    // next.tv_nsec = nanoseconds;
}

long getSecondsSinceStart() {
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return now.tv_sec- start.tv_sec;
}


void printTimeControl() {
    printf("time        sec    m  u  n\n");
    printf("start :%9li.%09li\n", start.tv_sec, start.tv_nsec);
    printf("now   :%9li.%09li\n", now.tv_sec, now.tv_nsec);
    printf("next  :%9li.%09li\n", next.tv_sec, next.tv_nsec);
}

struct timespec getNow() {
    return now;
}

double turnTime(struct timespec time) {
    return (time.tv_sec) + (long double)(time.tv_nsec)/1000000000;
}






/*int initTest() {

    int  mem_fd = open("/dev/mem", O_RDWR|O_SYNC); // open /dev/mem
    if (mem_fd < 0) {
        printf("can't open /dev/mem \n");
        return 1;
    }

    // mmap ARM Timer
    ARMTimer = mmap(
        NULL,             //Any adddress in our space will do
        BLOCK_SIZE,       //Map length
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        ARM_TIMER_CTL//Offset to ARM timer
        );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (ARMTimer == MAP_FAILED) {
        printf("mmap error 0x%08x\n", (uint32_t)ARM_TIMER_CTL);//errno also set!
        return 1;
    }


    printf("+0 %x\n", *(ARMTimer));

    // printf("+0 %x\n", *(ARMTimer));
    // printf("+7 %x\n", *(ARMTimer+7));
    // printf("+8 %x\n", *(ARMTimer+8));

    return 0;
}*/



int initARMTimer() {

    int  mem_fd = open("/dev/mem", O_RDWR|O_SYNC); // open /dev/mem
    if (mem_fd < 0) {
        printf("can't open /dev/mem \n");
        return 1;
    }

    /* mmap ARM Timer */
    ARMTimer = mmap(
        NULL,             //Any adddress in our space will do
        BLOCK_SIZE,       //Map length
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        ARM_QA7_CONTROL_REG//Offset to ARM timer
        );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (ARMTimer == MAP_FAILED) {
        printf("mmap error 0x%08x\n", (uint32_t)ARM_QA7_CONTROL_REG);//errno also set!
        return 1;
    }

    // printf("+0 %x\n", *(ARMTimer));
    // printf("+7 %x\n", *(ARMTimer+7));
    // printf("+8 %x\n", *(ARMTimer+8));

    return 0;
}


/*void resetTestTime() {
    lsbARMTimer = 0;
    msbARMTimer = (*(ARMTimer+CLOCK_MSB) + 1);
    *(ARMTimer+CLOCK_LSB) = lsbARMTimer;
    *(ARMTimer+CLOCK_MSB) = msbARMTimer;
}*/

void updateARMTimer() {
    lsbARMTimer = *(ARMTimer+CLOCK_LSB);
    msbARMTimer = *(ARMTimer+CLOCK_MSB);
}

void getARMTimer(unsigned long int *lsbTime, unsigned long int *msbTime) {
    updateARMTimer();
    *lsbTime = lsbARMTimer;
    *msbTime = msbARMTimer;
}

void setNextTime(unsigned long int lsbTime, unsigned long int msbTime) {
    lsbNextTime = lsbTime;
    msbNextTime = msbTime;
}

int isARMTimerLessThanNext() { //return 1 if now < next
    if(msbARMTimer != msbNextTime) {
        return (msbARMTimer < msbNextTime);
    }
    return lsbARMTimer < lsbNextTime;
}
