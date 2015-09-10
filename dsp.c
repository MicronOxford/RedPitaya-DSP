#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include "timer.h"
#include "rpouts.h"
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
	unsigned long long clocks;
	int pin;
	uint32_t parameter;
	struct actionTable *next;
} actionTable_t;


int readActionTable(FILE *fp, long lines);
int readActionTableLine(char *line, long lineno);
int execActionTable(long lines);
void sig_handler(int signo);
void _exit(int status);

void spinwait(int loops) {
	int t = 0;
	while (t != loops)
		t += 1;
}

actionTable_t *table = NULL;

int main(int argc, char *argv[])
{
	printf("hello, w\n");

	if (initTimer() == 1){
		printf("init timer failed\n");
		_exit(2);
	};

	if(initOuts() < 0){
		fprintf(stderr, "Rp api init failed!\n");
		_exit(2);
	}

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

	long lines = 0;
	int ch;
	while (EOF != (ch=getc(fp)))
	    if (ch=='\n')
	        ++lines;
	printf("actiontable is %lu lines long\n", lines);
	rewind(fp);

	printf("alloc action table\n");
	table = malloc(sizeof(actionTable_t)*lines);

	if (readActionTable(fp, lines) != 0){
		printf("Failed to read action table file.\n");
		_exit(3);
	}
	printf("read action table file.\n");

	// before we set ourselfves to more important than the terminal,
	// flush.
	fflush(stdout);

	struct sched_param params;
	params.sched_priority = 99;
	if (sched_setscheduler(0, SCHED_FIFO, &params) == -1){
		printf("Failed to set priority.\n");
		_exit(4);
	}

	int execstatus = execActionTable(lines);
	if (execstatus < 0) {
		printf("Failed to exec action table (%i).\n", execstatus);
		_exit(4);
	}
	printf("exec action table done.\n");

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
			printf("readActionTableLine failed on %lu\n", lineno);
			free(linestr);
			return -1;
		}
		//printf("malloc for line %i, addr %p\n", lines_read, nextRow);
		//printf("done  \n");
	}
	free(linestr);
	return 0;
}

int readActionTableLine(char *line, long lineno){

	char *nstime_s;
	char *pin_s;
	char *parameter_s;

	nstime_s = strtok(line, DELIM);  // REMINDER: first strtok call needs the str.
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

	table[lineno].clocks = strtoull(nstime_s, NULL, 10) * 1000000000L / COUNTS_PER_SECOND;
	table[lineno].pin = strtol(pin_s, NULL, 10);
	table[lineno].parameter = strtol(parameter_s, NULL, 10);
	//printf("row: %lu time:%llu pin:%i high:%i\n", lineno, table[lineno].nanos, table[lineno].pin, table[lineno].parameter);
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

	/* approx. jitter caused by clock_gettime and the context switch is about 500ns
	 * 10x worse than cycle counting.
	 * peak freq. obtainable using this is 270KHz - 2us bwteen signal events.
	 * need to be able to toggle pins simultainusly.
	 * jitter from nanos comp - about 25us bleh - 1 jiffy?
	 */
	printf("faffing with actiontables\n");
	XTime now;

	printf("set time\n");

	// while (1){
	// 	out_setpins(~out_getpins());
	// 	XTime_GetTime(&now);
	// }

	long line;
	XTime_SetTime(0);
	XTime_GetTime(&now);
	for (line = 0; line < lines; line++){
		while (now  <= table[line].clocks) XTime_GetTime(&now);
		out_setpins(table[line].pin);
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
	out_setpins(0);
	exit(status);
}
