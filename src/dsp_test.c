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
#include "gpioCont.h"

#define MAXLINELEN 50
#define MAXHANDLERLEN 5
#define DELIM " "
#define BILLION  1000000000L
#define PRINT_READ_LINES false
#define PRINT_EXEC_LINES true
#define TIMEDIV 100
#define MAXTESTTIME 10
#define TESTDIFTIME 0.001


char ERRVAL[] = "/n";

uint32_t INT_MAX = UINT32_MAX;

typedef struct actionTable {
    long double actionTime;
    int pinP;
    int pinN;
    uint32_t a1;
    uint32_t a2;
    double executedTime;
} actionTable_t;


int readActionTable(FILE *fp, long lines);
int readActionTableLine(char *line, long lineno);
int execTestTimer(long timeInt);
int execActionTable(long lines);
void sig_handler(int signo);
void _exit(int status);

/*void spinwait(int loops) {
    int t = 0;
    while (t != loops)
        t += 1;
}*/

actionTable_t *table = NULL;


int main(int argc, char *argv[]) {
    printf("hello, world!\n");

    if(initGPIO() != 0){
        printf("init GPIO access failed\n");
        _exit(2);
    }

    /*struct sched_param params;
    params.sched_priority = 99;
    if (sched_setscheduler(0, SCHED_FIFO, &params) == -1){
      printf("Failed to set priority.\n");
      _exit(4);
    }

    while(true) {
        signal9(1);
        usleep(1);
        signal9(0);
        usleep(1);
    }*/

    /*if (initTimer() == 1){
        printf("init timer failed\n");
        _exit(2);
    }*/

    /*if(initOuts() != 0){
        fprintf(stderr, "Rp api init failed!\n");
        _exit(2);
    }*/

    /*if(fpga_awg_init() != 0){
        fprintf(stderr, "FPGA init failed!\n");
        _exit(2);
    }*/

    if (signal(SIGINT, sig_handler) == SIG_ERR){
        printf("Signal handler failed\n");
    }


    if (argc != 2) {
        printf("num of args used was %u\n", argc);
        printf("USAGE: dsp ACTION_TABLE_FILE\n");
        _exit(1);
    }

    FILE *fp;
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Could not open file %s\n", argv[1]);
        _exit(2);
    }

    long lines = 0;
    int ch;
    while (EOF != (ch=getc(fp)))
        if (ch=='\n')
            ++lines;
       printf("action table is %lu lines long\n", lines);
       rewind(fp);

       printf("alloc action table\n");
       table = malloc(sizeof(actionTable_t)*lines);

       if (readActionTable(fp, lines) != 0){
          printf("Failed to read action table file.\n");
          _exit(3);
      }
      printf("read action table file.\n");

    // before we set ourselves to more important than the terminal, flush.
      fflush(stdout);
      
      struct sched_param params;
      params.sched_priority = 99;
      if (sched_setscheduler(0, SCHED_FIFO, &params) == -1){
          printf("Failed to set priority.\n");
          _exit(4);
      }

      printf("starting exec test.\n");
      execTestTimer(TESTDIFTIME);
      printf("exec test done.\n");
      /*int execstatus = execActionTable(lines);
      if (execstatus < 0) {
          printf("Failed to exec action table (%i).\n", execstatus);
          _exit(5);
      }
      printf("exec action table done.\n");*/

      _exit(0);
}

/******************************************************************************/

  int readActionTable(FILE *fp, long lines) {
   int bytes_read = 0;
   long lineno = 0;
   size_t linelen = (size_t)MAXLINELEN;
   char *linestr;
   linestr = (char *) malloc(sizeof(char)*(linelen+1));

   for (lineno = 0; lineno < lines; lineno++){
      bytes_read = getline(&linestr, &linelen, fp);
      if (bytes_read <= 0){
         printf("read %lu lines of action table, but was empty\n", lineno);
         free(linestr);
         return -1;
     }
     if (readActionTableLine(linestr, lineno) < 0){
         printf("read ActionTableLine failed on %lu\n", lineno);
         free(linestr);
         return -1;
     }
        //printf("malloc for line %i, addr %p\n", lines_read, nextRow);
        //printf("done  \n");
 }
 free(linestr);
 return 0;
}

int readActionTableLine(char *line, long lineNum) {

    char *nstime_s;
    char *pinP_s;
    char *pinN_s;
    char *a1_s;
    char *a2_s;

    //printf("processing line num %lu - '%s", lineNum, line);

    nstime_s = strtok(line, DELIM);  // REMINDER: first strtok call needs the str.
    if (nstime_s == NULL){
        printf("action nstime is NULL for '%s'\n", line);
        return -1;
    }
    pinP_s = strtok(NULL, DELIM);
    if (pinP_s == NULL){
        printf("pinP is NULL for %s\n", line);
        return -1;
    }
    pinN_s = strtok(NULL, DELIM);
    if (pinN_s == NULL){
        printf("pinN is NULL for %s\n", line);
        return -1;
    }
    a1_s = strtok(NULL, DELIM);
    if (a1_s == NULL){
        printf("a1_s is NULL for %s", line);
        return -1;
    }
    a2_s = strtok(NULL, DELIM);
    if (a2_s == NULL){
        printf("a2_s is NULL for %s", line);
        return -1;
    }
    //table[lineNum].clocks = (strtoull(nstime_s, NULL, 10) * COUNTS_PER_SECOND) / 1000000000L;
    table[lineNum].actionTime = strtold(nstime_s, NULL);
    table[lineNum].pinP = strtol(pinP_s, NULL, 10);
    table[lineNum].pinN = strtol(pinN_s, NULL, 10);
    table[lineNum].a1 = strtol(a1_s, NULL, 10);
    table[lineNum].a2 = strtol(a2_s, NULL, 10);
    if(PRINT_READ_LINES) {
        printf("row: %lu time:%Lf pinP:%i pinN:%i a1:%i a2:%i\n", lineNum, table[lineNum].actionTime, table[lineNum].pinP, table[lineNum].pinN, table[lineNum].a1, table[lineNum].a2);
    }
    return 0;
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
    /*int pinP = table[line].pinP;
    int pinN = table[line].pinN;
    uint32_t a1 = table[line].a1;
    uint32_t a2 = table[line].a2;
    printf("executed row: %lu pinP:%i pinN:%i a1:%i a2:%i\n", line, pinP, pinN, a1, a2);*/
    signal9(1);
    signal9(0);
}


int execTestTimer(long timeInt) {

    printf("set time\n");
    struct timespec start;
    clock_getres(CLOCK_MONOTONIC_RAW, &start);
    printf("The clock's resolution (precision) is %lu second(s) and %lu nanoseconds\n", start.tv_sec, start.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC_RAW, &start); //TODO check if return val is dif from -1 (error values and stuff)
    struct timespec now;
    double deltaT;
    double nextTime = timeInt;

    do {
        do {
            clock_gettime(CLOCK_MONOTONIC_RAW, &now);
            deltaT = (now.tv_sec - start.tv_sec) + (long double)(now.tv_nsec - start.tv_nsec)/1000000000;
        } while (deltaT  <= nextTime);

        signalChg9();

        nextTime += timeInt;

    } while(deltaT < MAXTESTTIME);

    return 0;
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
    struct timespec start;
    clock_getres(CLOCK_MONOTONIC_RAW, &start);
    printf("The clock's resolution (precision) is %lu second(s) and %lu nanoseconds\n", start.tv_sec, start.tv_nsec);
    clock_gettime(CLOCK_MONOTONIC_RAW, &start); //TODO check if return val is dif from -1 (error values and stuff)
    struct timespec now;
    //clock_gettime(CLOCK_MONOTONIC_RAW, &now);

    for (line = 0; line < lines; line++){
        double deltaT;

        //printf("now - i: %i , sec: %li , nano: %li\n", nowI, now.tv_sec, now.tv_nsec);
        //deltaT = (now.tv_sec - start.tv_sec) + (long double)(now.tv_nsec - start.tv_nsec)/1000000000;
        //printf( "%lf\n", deltaT);
        //int asd = frequency_of_primes(99999); printf( "%u\n", asd);

        do {
            clock_gettime(CLOCK_MONOTONIC_RAW, &now);
            deltaT = (now.tv_sec - start.tv_sec) + (long double)(now.tv_nsec - start.tv_nsec)/1000000000;
            //printf( "%lf %Lf\n", deltaT, table[line].actionTime);
        } while (deltaT  <= table[line].actionTime);

        if(table[line].pinP == -1 && table[line].pinP == -1) {

            while(!getCameraReady()) {
                //IT'S ADVENTURE TIME
            }
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
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
            //printf("line %lu with time %Lf (diff of %lu) was executed at time %lf\n", line, table[line].actionTime, table[line].executedTime, difTime);
            difTime = table[line].executedTime - table[line].actionTime;
            totalDif += difTime;
            printf("line %lu with time %Lf (diff of %lf) was executed at time %lf\n", line, table[line].actionTime, table[line].executedTime, difTime);
            if(difTime > maxVal) { maxVal = difTime; }
            if(difTime < minVal) { minVal = difTime; }
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
