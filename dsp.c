#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include "timer.h"

//#include "rp.h"

#define MAXLINELEN 50
#define MAXHANDLERLEN 5
#define DELIM " "
#define BILLION  1000000000L;

char ERRVAL[] = "/n";

/*
int ktime_get(void){
	return 0;
}

typedef int ktime_t;
*/

uint32_t INT_MAX = UINT32_MAX;

typedef struct actionTable {
	long secs;
	long nanos;
	int pin;
	uint32_t parameter;
	struct actionTable *next;
} actionTable_t;


int readActionTable(FILE *fp);
int readActionTableLine(char *line, actionTable_t *tableEntry);
int execActionTable();
void sig_handler(int signo);
void _exit(int status);

void spinwait(int loops) {
	int t = 0;
	while (t != loops)
		t += 1;
}

actionTable_t *table;

int main(int argc, char *argv[])
{
	printf("hello, w\n");

	if (initTimer() == 1){
		_exit(2);
	};
	printf("init timer\n");

	// if(rp_Init() != RP_OK){
	// 	fprintf(stderr, "Rp api init failed!\n");
	// }

	if (signal(SIGINT, sig_handler) == SIG_ERR){
  	printf("Signal handler failed\n");
  }

	printf("hello, world!\n");

	/* jitter is about 2us with peaks of 40us */
	/* with spinwait, jitter is approx 100ns peak
     * as long as it does not accumulate this is probably ok
     */

	if (argc != 2) {
		printf("USAGE: dsp ACTIONTABLEFILE\n");
		_exit(1);
	}

	FILE *fp;
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Could not open file %s\n", argv[1]);
		_exit(2);
	}

	table = (actionTable_t *) malloc(sizeof(actionTable_t));

	if (readActionTable(fp) != 0){
		printf("Failed to read action table file.\n");
		_exit(3);
	}
	printf("read action table file.\n");

	struct sched_param params;
	params.sched_priority = 99;
	if (sched_setscheduler(0, SCHED_FIFO, &params) == -1){
		printf("Failed to set priority.\n");
		_exit(4);
	}

	int execstatus = execActionTable();
	if (execstatus < 0) {
		printf("Failed to exec action table (%i).\n", execstatus);
		_exit(4);
	}
	printf("exec action table done.\n");

	_exit(0);
}

/******************************************************************************/

int readActionTable(FILE *fp) {
	int bytes_read = 0;
	int lines_read = 0;
	size_t linelen = (size_t)MAXLINELEN;
	char *line;
	line = (char *) malloc(sizeof(char)*(linelen+1));
	actionTable_t *currRow = table;
	actionTable_t *nextRow;

	while (1) {
		bytes_read = getline(&line, &linelen, fp);
		if (bytes_read <= 0){
			printf("read %i lines of action table\n", lines_read);
			return 0;
		}
		if (readActionTableLine(line, currRow) < 0){
			printf("readActionTableLine failed on %i\n", lines_read);
			return -1;
		}
		nextRow = (actionTable_t *)malloc(sizeof(actionTable_t));
		//printf("malloc for line %i, addr %p\n", lines_read, nextRow);
		nextRow->next = (actionTable_t *)0;
		currRow->next = nextRow;
		currRow = nextRow;
		//printf("done  \n");
		lines_read += 1;
	}
}

int readActionTableLine(char *line, actionTable_t *tableEntry){
	char *stime_s;
	char *nstime_s;
	char *pin_s;
	char *parameter_s;

	stime_s = strtok(line, DELIM);
	if (stime_s == NULL){
		printf("action stime is NULL for '%s'\n", line);
		return -1;
	}
	nstime_s = strtok(NULL, DELIM);
	if (nstime_s == NULL){
		printf("action nstime is NULL for '%s'\n", line);
		return -1;
	}
	pin_s = strtok(NULL, DELIM);
	if (pin_s == NULL){
		printf("pin is NULL for %s\n", line);
		return -1;
	}
	parameter_s = strtok(NULL, DELIM);
	if (parameter_s == NULL){
		printf("parameter_s is NULL for %s", line);
		return -1;
	}

	tableEntry->secs = strtoll(stime_s, NULL, 10);
	tableEntry->nanos = strtoll(nstime_s, NULL, 10);
	// tableEntry->pin = (rp_dpin_t)strtol(pin_s, NULL, 10);
	tableEntry->parameter = strtol(parameter_s, NULL, 10);
	//printf("time:%lli pin:%i high:%i\n", tableEntry->time, tableEntry->pin, tableEntry->parameter);
	return 0;
}

/* need to add error handling on the clock */
int execActionTable() {
	struct timespec start, now;
	long currsec = 0;
	long currnano = 0;
	long nanooffset;
	long secsoverdue = -1;
	long nsoverdue = -1;

	float a_volts = 0;
  // rp_GenWaveform(RP_CH_1, RP_WAVEFORM_DC);
	// rp_GenWaveform(RP_CH_2, RP_WAVEFORM_DC);
	// rp_GenAmp(RP_CH_1, a_volts);
	// rp_GenAmp(RP_CH_2, a_volts);
	// rp_GenOutEnable(RP_CH_1);
	// rp_GenOutEnable(RP_CH_2);

	/* approx. jitter caused by clock_gettime and the context switch is about 500ns
	 * 10x worse than cycle counting.
	 * peak freq. obtainable using this is 270KHz - 2us bwteen signal events.
	 * need to be able to toggle pins simultainusly.
	 * jitter from nanos comp - about 25us bleh - 1 jiffy?
	 */
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &start) == -1)
		return -1;
	nanooffset = start.tv_nsec;
	actionTable_t *currRow = table;
	while (1) {
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &now) == -1)
			return -1;
		currsec = (long)now.tv_sec-(long)start.tv_sec;
		if (currsec == 0){
			currnano = now.tv_nsec-nanooffset;
		} else {
			currnano = now.tv_nsec;
		}
		secsoverdue = currsec - currRow->secs;
		nsoverdue = currnano - currRow->nanos;
		// could possibly loop in the ns part once we are in the right second, to reduce wait time/jitter?
		if (secsoverdue > 0 || (secsoverdue == 0 && nsoverdue > 0)){
			if (currRow->pin >= 0){
			  // rp_DpinSetState(currRow->pin, currRow->parameter);
			} else {
				// Analog outs are reprsented by negative pin nums
				// and the voltage by a uint32_t
				// convert channel from [-1, -2] to [0, 1]
				// and 2**31 to 0.5
				// rp_GenAmp((currRow->pin*-1)-1, (float)currRow->parameter/INT_MAX);
			}
			if (currRow->next != NULL) {
				currRow = currRow->next;
			} else {
				return 0;
				//clock_gettime(CLOCK_MONOTONIC_RAW, &start);
				//currRow = table;
			}
		}
	}
}

void sig_handler(int signo){
  if (signo == SIGINT)
    _exit(5);
}

void _exit(int status) {
	actionTable_t *next = table;
	while (next != NULL) {
		next = table->next;
		free(table);
		table = next;
	}
	// rp_GenAmp(RP_CH_1, 0);
	// rp_GenAmp(RP_CH_2, 0);
	// rp_ApinReset();
	// rp_DpinReset();
	// rp_Release();
	exit(status);
}
