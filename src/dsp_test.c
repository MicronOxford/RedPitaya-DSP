#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <math.h>
//#include "timer.h"
//#include "rpouts.h"
//#include "fpga_awg.h"
#include "gpioControl.h"
#include "timeControl.h"
#include "actionTable.h"

// #define MAXLINELEN 50
// #define MAXHANDLERLEN 5
// #define DELIM " "
// #define PRINT_READ_LINES false
#define PRINT_EXEC_LINES true
#define TIMEDIV 100
//#define MAXTESTTIME 10
//#define TESTNANOSEC 1000

#define TESTARMNUMBER 100000
#define TESTARMNSECON 2000
#define TESTARMNSECOFF 3000
//#define TESTMICROSEC 1
/*

ToDo
check jitter, changing the setting of GPIO back to GPIO, but pre setting and activate the setiing after

change table to an actual table (instead of test table)
saving the current time

waiting states
*/

char ERRVAL[] = "/n";

//uint32_t INT_MAX = UINT32_MAX;


// typedef struct testTable {
//     struct timespec actTime;
//     struct timespec execTime;
// } testTable_t;


typedef struct testARMTimerTable {
    uint64_t nextTime;
    int val;
} testARMTimerTable_t;



void initializeAll();
void maxPriority();
int execActionTable(long lines);
void sig_handler(int signo);
void _exit(int status);

int execTestTimer(long nanoSecInt);

void execARMTimerTest();


//actionTable_t *table = NULL;

//testTable_t *testTbl = 0;
testARMTimerTable_t *testARMTbl = 0; 


int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("num of args used was %u\n", argc);
        printf("USAGE: dsp ACTION_TABLE_FILE\n");
        _exit(1);
    }

    initializeAll();

    long int lines = createActionTable(argv[1]);
    if(lines < 0) _exit(abs(lines));

    // before we set ourselves to more important than the terminal, flush.
    fflush(stdout);

    maxPriority();

    /*printf("starting exec test.\n");
    execARMTimerTest();
    printf("exec test done.\n");*/
    int execStatus = execActionTable(lines);
    if (execStatus < 0) {
        printf("Failed to exec action table (%i).\n", execStatus);
        _exit(5);
    }
    printf("exec action table done.\n");
    _exit(0);
}

/******************************************************************************/

void initializeAll() {

    if(initGPIO() != 0){
        printf("init GPIO access failed\n");
        _exit(2);
    }

    if(initARMTimer() != 0){
        printf("init ARM Timer access failed\n");
        _exit(2);
    }

    // if (initTimer() == 1){
    //     printf("init timer failed\n");
    //     _exit(2);
    // }

    // if(initOuts() != 0){
    //     fprintf(stderr, "Rp api init failed!\n");
    //     _exit(2);
    // }

    // if(fpga_awg_init() != 0){
    //     fprintf(stderr, "FPGA init failed!\n");
    //     _exit(2);
    // }

    if (signal(SIGINT, sig_handler) == SIG_ERR){
        printf("Signal handler failed\n");
    }

}

void maxPriority() {
    struct sched_param params;
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (sched_setscheduler(0, SCHED_FIFO, &params) == -1){
        printf("Failed to set priority.\n");
        _exit(4);
    }
}

bool getCameraReady() {
    return true;
}

void executeAction(long line) {
    /*int pinP = table[line].pinP;
    int pinN = table[line].pinN;
    uint32_t a1 = table[line].a1;
    uint32_t a2 = table[line].a2;
    printf("executed row: %lu pinP:%i pinN:%i a1:%i a2:%i\n", line, pinP, pinN, a1, a2);*/
    printf("executed row: %lu pin:%i action:%i\n", line, actionTable[line].pin, actionTable[line].action);
    // signal9(1);
    // signal9(0);
}

/* need to add error handling on the clock */
int execActionTable(long lines) {
    printf("exec action table\n");
    // float a_volts = 0;
    // rp_GenWaveform(RP_CH_1, RP_WAVEFORM_DC);
    // rp_GenWaveform(RP_CH_2, RP_WAVEFORM_DC);
    // rp_GenAmp(RP_CH_1, a_volts);
    // rp_GenAmp(RP_CH_2, a_volts);
    // rp_GenOutEnable(RP_CH_1);
    // rp_GenOutEnable(RP_CH_2);

    //printf("faffing with actiontables\n");

    /*clock_t clockT = clock();
    time_t timeT = time(NULL);

    printf("starting clock is %li, while start time is %li\n", clockT, timeT);

    for(unsigned int i=0; i<100000; ++i) {
        //printf("the dif time in it %u is - clock: %f ; time: %li\n", i, ((float)(clock()-clockT))/CLOCKS_PER_SEC, (time(NULL)-timeT));
        //printf("the dif time in it %u is - clock: %li ; time: %li\n", i, (clock()-clockT), (time(NULL)-timeT));
        printf("the dif time in it %u is - clock: %f\n", i, ((float)(clock()-clockT))/CLOCKS_PER_SEC );
    }
    printf("test %li\n", CLOCKS_PER_SEC);*/

    //XTime now;
    //XTime_SetTime(0);
    //XTime_GetTime(&now);

    long line = 0;
    startARMTimer();

    for ( ; line < lines; line++){

        setNextTime(actionTable[line].actionTime);

        if(actionTable[line].action == -1) {
            waitForNext();
            while(!getCameraReady()) {
                //IT'S ADVENTURE TIME
            }
            startARMTimer();
        } else {
            waitForNext();
            *(actionTable[line].pinAddr) = actionTable[line].valToWrit;
        }

        

        /*setNextTime(table[line].actionTime);

        while(isARMTimerLessThanNext());

        if(table[line].pinP == -1 && table[line].pinP == -1) {
            while(!getCameraReady()) {
                //IT'S ADVENTURE TIME
            }
            startARMTimer();
            printf("TIME RESET\n");
        }

        executeAction(line);*/
        //table[line].executedTime = deltaT;
    }


    //(((X) < (Y)) ? (X) : (Y))

    /*if(PRINT_EXEC_LINES) {
        double difTime = 0.0;
        double totalDif = 0.0;
        double maxVal = 0.0;
        double minVal = 99.9;
        for(line = 0; line <lines; line++) {
            difTime = table[line].executedTime - table[line].actionTime;
            totalDif += difTime;
            if(difTime > maxVal) { maxVal = difTime; }
            if(difTime < minVal) { minVal = difTime; }
            printf("line %lu with time %Lf (diff of %lf) was executed at time %lf\n", line, table[line].actionTime, table[line].executedTime, difTime);
        }
        printf("The sum/max/min of difference between executedTime and actionTime was %lf / %lf / %lf\n", totalDif, maxVal, minVal);
    }*/

    return 0;
}

void sig_handler(int signo){
    if (signo == SIGINT) {
        _exit(5);
    }
}

void _exit(int status) {
    //free(table);
    free(actionTable);
    free(testARMTbl);
    // rp_GenAmp(RP_CH_1, 0);
    // rp_GenAmp(RP_CH_2, 0);
    // rp_ApinReset();
    // rp_DpinReset();
    // rp_Release();
    /*out_setpins_P(0); //TODO activate this (uncoment)
    out_setpins_N(0);
    fpga_awg_exit();*/
    exit(status);
}


/******************************************************************************/

void createARMTimerTestTable(long numOfInt){

    printf("alloc test table2\n");
    testARMTbl = malloc(sizeof(testARMTimerTable_t)*numOfInt);

    // unsigned long int myARMTime;
    // getARMTimer(&myARMTime);

    long i;
    unsigned long long int timeToSet = BILLION;
    double nSec2TckRatio = getNSecPerTick();
    //unsigned long int inc = nanoInt/nSec2TckRatio;

    for(i=0; i<numOfInt; i++) {
        if(i%2) {
            timeToSet += TESTARMNSECON;
            testARMTbl[i].val = 0;
        } else {
            timeToSet += TESTARMNSECOFF;
            testARMTbl[i].val = 1;
        }
        testARMTbl[i].nextTime = (uint64_t)(timeToSet/nSec2TckRatio);
        // testARMTbl[i].nextTime = myARMTime + timeToSet/nSec2TckRatio;
    }
}

void execARMTimerTest() {

    printf("creating ARMTimer test table\n");
    long numOfInt = TESTARMNUMBER;
    createARMTimerTestTable(numOfInt);
    // printf("setting time\n");
    // resetTestTime();

    // unsigned long int myLSBTime;
    // unsigned long int myMSBTime;
    // getARMTimer(&myLSBTime, &myMSBTime);

    volatile uint32_t *toSendAddr;
    uint32_t toSendValue;
    
    // setSignal9(i, &addr, &val);

    startARMTimer();

    long i;
    for(i=0; i<numOfInt; i++){
        setNextTime(testARMTbl[i].nextTime);
        setSignal9(testARMTbl[i].val, &toSendAddr, &toSendValue);

        while(isARMTimerLessThanNext()) {

        }

        *(toSendAddr) = toSendValue;
        // signal9(testARMTbl[i].val);
        // signalChg9();
        // printf("TEST\n");
    }

    signalOFF9();
    // printf("DONE2\n");
}