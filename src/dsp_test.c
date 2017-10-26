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
#define MAXTESTTIME 10
#define TESTNANOSEC 1000
#define TEST2NUMTIMES 100
//#define TESTMICROSEC 1


char ERRVAL[] = "/n";

//uint32_t INT_MAX = UINT32_MAX;


typedef struct testTable {
    struct timespec actTime;
    struct timespec execTime;
} testTable_t;


typedef struct testTable2 {
    long unsigned int lsbNextTime;
    long unsigned int msbNextTime;
} testTable_t2;



void initializeAll();
int execActionTable(long lines);
void sig_handler(int signo);
void _exit(int status);

int execTestTimer(long nanoSecInt);


void execTestTimer2();


//actionTable_t *table = NULL;

testTable_t *testTbl = 0;
testTable_t2 *testTbl2 = 0; 


int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("num of args used was %u\n", argc);
        printf("USAGE: dsp ACTION_TABLE_FILE\n");
        _exit(1);
    }

    initializeAll();

    long lines = createActionTable(argv[1]);
    if(lines < 0) _exit(abs(lines));

    // before we set ourselves to more important than the terminal, flush.
    fflush(stdout);

    struct sched_param params;
    params.sched_priority = 99;
    if (sched_setscheduler(0, SCHED_FIFO, &params) == -1){
        printf("Failed to set priority.\n");
        _exit(4);
    }

    printf("starting exec test.\n");
    execTestTimer2();
    printf("exec test done.\n");
    // int execstatus = execActionTable(lines);
    // if (execstatus < 0) {
    //     printf("Failed to exec action table (%i).\n", execstatus);
    //     _exit(5);
    // }
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
        printf("init CLOCK access failed\n");
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

bool getCameraReady() {
    return true;
}

int frequency_of_primes (int n) {
    int i,j;
    int freq=n-1;

    for (i=2; i<=n; ++i) {
        for (j=i-1;j>1;--j){
            if (i%j==0) {--freq; break;}
        }
    }
    return freq;
} //TODO remove this

void executeAction(long line) {
    int pinP = table[line].pinP;
    int pinN = table[line].pinN;
    uint32_t a1 = table[line].a1;
    uint32_t a2 = table[line].a2;
    printf("executed row: %lu pinP:%i pinN:%i a1:%i a2:%i\n", line, pinP, pinN, a1, a2);
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

    long line;

    printf("!!!REMOVE THIS!!! reducing the action time. !!!REMOVE THIS!!!\n");
    for (line = 0; line < lines; line++) {
        table[line].actionTime /= TIMEDIV;
    }


    printf("set time\n");
    startTime();

    for (line = 0; line < lines; line++){
        double deltaT;

        //printf("now - i: %i , sec: %li , nano: %li\n", nowI, now.tv_sec, now.tv_nsec);
        //deltaT = (now.tv_sec - start.tv_sec) + (long double)(now.tv_nsec - start.tv_nsec)/1000000000;
        //printf( "%lf\n", deltaT);
        //int asd = frequency_of_primes(99999); printf( "%u\n", asd);

        do {
            deltaT = getTimeSinceStart();
            //printf( "%lf %Lf\n", deltaT, table[line].actionTime);
        } while (deltaT  <= table[line].actionTime);

        if(table[line].pinP == -1 && table[line].pinP == -1) {
            while(!getCameraReady()) {
                //IT'S ADVENTURE TIME
            }
            startTime();
            printf("TIME RESET\n");
        }

        executeAction(line);
        table[line].executedTime = deltaT;
    }

    //(((X) < (Y)) ? (X) : (Y))

    if(PRINT_EXEC_LINES) {
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
    }

    return 0;
}

void sig_handler(int signo){
    if (signo == SIGINT)
        _exit(5);
}

void _exit(int status) {
    free(table);
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

long createTestTable(long nanoInt, long numOfInt){

    printf("alloc test table\n");
    testTbl = malloc(sizeof(testTable_t)*numOfInt);

    long i;
    struct timespec testTime;
    clock_gettime(CLOCK_MONOTONIC_RAW, &testTime);
    testTime.tv_sec++;
    for(i=0; i<numOfInt; i++) {
        testTbl[i].actTime = testTime;
        addNanoSecToTime(nanoInt, &testTime);
    }

    // if(readActionTable(fp, lines) != 0) {
    //     printf("Failed to read action table file.\n");
    //     return -3;
    // }
    // printf("read action table file.\n");

    return 0;
}

int execTestTimer(long nanoSecInt) {

    printf("creating test table\n");
    long numOfInt = 100;
    createTestTable(nanoSecInt, numOfInt);
    printf("setting time\n");
    startTime();
    struct timespec time;

    long i;
    for(i=0; i<numOfInt; i++){
        time = testTbl[i].actTime;
        while(compareTimeToNow(&time)) { /* sleep is overrated... */ }
        testTbl[i].execTime = getNow();
        //signalChg9();
        //printf("TEST\n");
    }

    //signalOFF9();
    printf("DONE\n");


    if(PRINT_EXEC_LINES) {
        double difTime = 0.0;
        double totalDif = 0.0;
        double maxVal = 0.0;
        double minVal = 99.9;
        for(i=0; i<numOfInt; i++) {
            double actionTime = turnTime(testTbl[i].actTime);
            double executedTime = turnTime(testTbl[i].execTime);
            difTime = executedTime - actionTime;
            totalDif += difTime;
            if(difTime > maxVal) { maxVal = difTime; }
            if(difTime < minVal) { minVal = difTime; }
            printf("line %lu with time %lf (diff of %lf) was executed at time %lf\n", i, actionTime, executedTime, difTime);
        }
        printf("The sum/max/min of difference between executedTime and actionTime was %9.9lf / %9.9lf / %9.9lf\n", totalDif, maxVal, minVal);
    }

    
    return 0;
}

/*int execTestTimerOLD(long nanoSecInt) {

    printf("set time\n");
    startTime();
    addNanoSecToNext(nanoSecInt);

    while(getSecondsSinceStart() < MAXTESTTIME) {

        while(compareNextToNow()) { }
        signalChg9();
        addNanoSecToNext(nanoSecInt);
    }

    signalOFF9();
    
    return 0;
}*/







void createTestTable2(long nanoInt, long numOfInt){

    printf("alloc test table2\n");
    testTbl2 = malloc(sizeof(testTable_t2)*numOfInt);

    long i;
    unsigned long int value = BILLION/NANO_PER_CLICK;
    unsigned long int inc = nanoInt/NANO_PER_CLICK;

    for(i=0; i<numOfInt; i++) {
        testTbl2[i].lsbNextTime = value;
        value += inc;
    }
}

void execTestTimer2() {

    printf("creating test table2\n");
    long numOfInt = TEST2NUMTIMES;
    createTestTable2(TESTNANOSEC, numOfInt);
    // printf("setting time\n");
    // resetTestTime();

    long i;
    for(i=0; i<numOfInt; i++){
        setNextTime(testTbl2[i].lsbNextTime, testTbl2[i].msbNextTime);

        while(isARMTimerLessThanNext()) {
            updateARMTimer();
        }
        signalChg9();
        // printf("TEST\n");
    }

    signalOFF9();
    // printf("DONE2\n");
}