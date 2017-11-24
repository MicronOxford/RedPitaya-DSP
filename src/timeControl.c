#include "timeControl.h"
//#include <time.h>
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
//uint64_t nowTime;


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

    return 0;
}


/*void resetTestTime() {
    lsbARMTimer = 0;
    msbARMTimer = (*(ARMTimer+CLOCK_MSB) + 1);
    *(ARMTimer+CLOCK_LSB) = lsbARMTimer;
    *(ARMTimer+CLOCK_MSB) = msbARMTimer;
}*/

void startARMTimer() {
    startTime = *(ARMTimer);
}

/*void updateARMTimer() {
    nowTime = *(ARMTimer);
}*/

void getARMTimer(uint64_t *utime) {
   *utime = *(ARMTimer);
}

void setNextTime(uint64_t nTime) {
    nextTime = startTime+nTime;
    // lsbNextTime = lsbTime;
    // msbNextTime = msbTime;
}

int isARMTimerLessThanNext() { //return 1 if now < next
   return *(ARMTimer) < nextTime;
    // if(msbARMTimer != msbNextTime) {
    //     return (msbARMTimer < msbNextTime);
    // }
    // return lsbARMTimer < lsbNextTime;
}

void waitForNext() {
    while(*(ARMTimer) < nextTime) { }
}

void printARMControl() {
    printf("ARM Control register %08i\n", (uint32_t)*(ARMControl));
}

void printARMTime() {
    uint64_t nowTime = *(ARMTimer);
    double nSec2TckRatio = getNSecPerTick();
    printf("start :%li  %li\n", (long int)startTime, (long int)(startTime*nSec2TckRatio) );
    printf("now   :%li  %li\n", (long int)nowTime, (long int)(nowTime*nSec2TckRatio) );
    printf("next  :%li  %li\n", (long int)nextTime, (long int)(nextTime*nSec2TckRatio) );
}
