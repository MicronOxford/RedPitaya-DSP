#include "timeControl.h"
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#define BLOCK_SIZE (4*1024)

// clock access
volatile uint32_t *ARMControl;
volatile uint64_t *ARMTimer;

uint64_t startTime;
uint64_t nextTime;
uint64_t currTime;

uint32_t *scanTime;

void testARMTimer();
inline void updateARMTimer();


double getNSecPerTick() {
    return BILLION/PROC_FREQ;
}

uint64_t turnNSecToTicks(unsigned long long int nSec) {
    return (uint64_t) (nSec/getNSecPerTick());
}

int initARMTimer() {

    int  mem_fd = open("/dev/mem", O_RDWR|O_SYNC); // open /dev/mem
    if (mem_fd < 0) {
        printf("can't open /dev/mem (errno %d) \n", errno);
        return 1;
    }

    /* mmap ARM Timer */
    ARMControl = mmap(
        NULL,                   //Any adddress in our space will do
        BLOCK_SIZE,             //Map length
        PROT_READ|PROT_WRITE,   // Enable reading & writting to mapped memory
        MAP_SHARED,             //Shared with other processes
        mem_fd,                 //File to map
        ARM_QA7_CONTROL_REG     //Offset to ARM control logic
        );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (ARMControl == MAP_FAILED) {
        printf("mmap(0x%08x) failed (errno %d)\n", (uint32_t)ARM_QA7_CONTROL_REG, errno);
        return 1;
    }

    ARMTimer = (uint64_t *)&ARMControl[ARM_TIMER_OFFSET]; //
    scanTime = (uint32_t *)&currTime;

    
    /*uint32_t lsbTimer = *(ARMTimer+7);
    uint32_t msbTimer = *(ARMTimer+8);
    printf("test0 %08x %08x\n", lsbTimer, msbTimer);

    lsbTimer = *(ARMTimer+7);
    sleep(300);
    msbTimer = *(ARMTimer+8);
    printf("test1 %08x %08x\n", lsbTimer, msbTimer);

    lsbTimer = *(ARMTimer+7);
    msbTimer = *(ARMTimer+8);
    printf("test2 %08x %08x\n", lsbTimer, msbTimer);

    testARMTimer();*/


    return 0;
}

void testARMTimer() {
    uint64_t previousVal = 0;
    uint64_t currentVal = 0;

    while(1) {
        updateARMTimer();
        currentVal = currTime;
        if(currentVal < previousVal) {
            printf("UPPSY DAISY!\n");
            printf("prev: %llu\n",(long long unsigned int) previousVal);
            printf("curr: %llu\n",(long long unsigned int) currentVal);
            return;
        }
        previousVal = currentVal;
    }

}

/*void resetTestTime() {
    lsbARMTimer = 0;
    msbARMTimer = (*(ARMTimer+CLOCK_MSB) + 1);
    *(ARMTimer+CLOCK_LSB) = lsbARMTimer;
    *(ARMTimer+CLOCK_MSB) = msbARMTimer;
}*/

void startARMTimer() {
    //startTime = *(ARMTimer);
    updateARMTimer();
    startTime = currTime;
}

inline void updateARMTimer() {
    do {
        currTime = *(ARMTimer);
    } while (*(scanTime)^0xFFFFFFFF);
}

/*void getARMTimer(uint64_t *utime) {
   *utime = *(ARMTimer);
}*/

void setNextTime(uint64_t nTime) {
    nextTime = startTime+nTime;
    // lsbNextTime = lsbTime;
    // msbNextTime = msbTime;
}

int isARMTimerLessThanNext() { //return 1 if now < next
    updateARMTimer();
    return currTime < nextTime;
    // return *(ARMTimer) < nextTime;
    // if(msbARMTimer != msbNextTime) {
    //     return (msbARMTimer < msbNextTime);
    // }
    // return lsbARMTimer < lsbNextTime;
}

void waitForNext() {
    //while(*(ARMTimer) < nextTime) { }
    updateARMTimer();
    while(currTime < nextTime) {
        updateARMTimer();
    }
}

uint64_t getCurrentTime() {
    return currTime;
}


void printARMControl() {
    printf("ARM Control register %08i\n", (uint32_t)*(ARMControl));
}

void printARMTime() {
    updateARMTimer();
    // uint64_t nowTime = *(ARMTimer);
    uint64_t nowTime = currTime;
    double nSec2TckRatio = getNSecPerTick();
    printf("start :%li  %li\n", (long int)startTime, (long int)(startTime*nSec2TckRatio) );
    printf("now   :%li  %li\n", (long int)nowTime, (long int)(nowTime*nSec2TckRatio) );
    printf("next  :%li  %li\n", (long int)nextTime, (long int)(nextTime*nSec2TckRatio) );
}
