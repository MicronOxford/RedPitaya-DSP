#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <errno.h>
#include "timer.h"
#include "rpouts.h"
#include "fpga_awg.h"

#define MAXLINELEN 50
#define MAXHANDLERLEN 5
#define DELIM " "
#define BILLION  1000000000L

char ERRVAL[] = "/n";

uint32_t INT_MAX = UINT32_MAX;

typedef struct actionTable {
	unsigned long long clocks;
	int pinP;
	int pinN;
	uint32_t a1;
	uint32_t a2;
} actionTable_t;

typedef enum {DISK, PIPE} atsource;

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

actionTable_t *table = NULL; //global so it can be free'd in exit.

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

	if(fpga_awg_init() != 0){
		fprintf(stderr, "FPGA init failed!\n");
		_exit(2);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR){
  	printf("Signal handler failed\n");
  }

	printf("hello, world!\n");

	atsource actionTableSource;
	if (argc == 2){
		actionTableSource = DISK;
	} else if (argc == 3 && strncmp(argv[1], "-", 1) == 0){ // we check that length is a num later
		actionTableSource = PIPE;
	} else {
		printf("USAGE: dsp ACTIONTABLEFILE or dsp - LENGTH\n");
		_exit(1);
	}

	long lines = 0;

	if (actionTableSource == DISK){

		FILE *fp;
		fp = fopen(argv[1], "r");
		if (fp == NULL) {
			printf("Could not open file %s\n", argv[1]);
			_exit(2);
		}

		lines = 0;
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

	} else {

		char *endptr; // we do not use this, but we need a bit of stack to store
									// the location of the remainder of the length arg
		errno = 0;    // so we can check that strtol succeeds
		lines = strtol(argv[2], &endptr, 0); //2nd arg is no lines
		if (errno != 0){
			printf("USAGE: dsp ACTIONTABLEFILE or dsp - LENGTH\n");
			_exit(1);
		}

		printf("alloc action table\n");
		long at_size = sizeof(actionTable_t)*lines;
		table = malloc(at_size);
		char *tablebuf = (char *)table;
		printf("reading %lu bytes form stdin\n", at_size);

		long count = 0;
		while (count < at_size){
			errno = 0;
			count += read(0, (char *)(tablebuf+count), at_size-count);
			if (errno != 0) perror("error reading from\n");
		}
		// for (long i = 0; i < at_size; i++){ // get the total number of bytes, write to
		// 																// table interpreted as a bytes array. C.
		// 	errno = 0;
		// 	tablebuf[i] = fgetc(stdin);
		// 	if (errno != 0){
		// 		perror("error writing to\n");
		// 	}
		// }
		printf("read action table from stdin.\n");
	}
	// before we set ourselves to more important than the terminal,
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
		_exit(5);
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
	char *pinP_s;
	char *pinN_s;
	char *a1_s;
	char *a2_s;

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
		printf("a1_s is NULL for %s\n", line);
		return -1;
	}
	a2_s = strtok(NULL, DELIM);
	if (a2_s == NULL){
		printf("a2_s is NULL for %s\n", line);
		return -1;
	}
	table[lineno].clocks = (strtoull(nstime_s, NULL, 10) * COUNTS_PER_SECOND) / BILLION;
	table[lineno].pinP = strtol(pinP_s, NULL, 10);
	table[lineno].pinN = strtol(pinN_s, NULL, 10);
	table[lineno].a1 = strtol(a1_s, NULL, 10);
	table[lineno].a2 = strtol(a2_s, NULL, 10);
	// printf("row: %lu time:%llu pinP:%i pinN:%i a1:%i\n", lineno, table[lineno].clocks, table[lineno].pinP, table[lineno].pinN, table[lineno].a1);
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

	printf("faffing with actiontables\n");
	XTime now;

	printf("set time\n");

	long line;
	XTime_SetTime(0);
	XTime_GetTime(&now);
	for (line = 0; line < lines; line++){
		while (now  <= table[line].clocks) XTime_GetTime(&now);
		out_setpins_P(table[line].pinP);
		out_setpins_N(table[line].pinN);
		fpga_awg_write_val_a(table[line].a1);
		fpga_awg_write_val_b(table[line].a2);
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
	out_setpins_P(0);
	out_setpins_N(0);
	fpga_awg_exit();
	exit(status);
}
