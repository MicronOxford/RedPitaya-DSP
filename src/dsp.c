/*
* Copyright (C) 2015 Tom Parks
*
* This file is part of RedPitaya-DSP.
*
* RedPitaya-DSP is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* RedPitaya-DSP is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with RedPitaya-DSP. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include "timer.h"
#include "rpouts.h"
#include "fpga_awg.h"

#define MAXLINELEN 50
#define MAXHANDLERLEN 5
#define DELIM " "
#define BILLION  1000000000L;

char ERRVAL[] = "/n";

uint32_t INT_MAX = UINT32_MAX;

typedef struct actionTable {
	unsigned long long clocks;
	int pinP;
	int pinN;
	uint32_t a1;
	uint32_t a2;
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

	if(fpga_awg_init() != 0){
		fprintf(stderr, "FPGA init failed!\n");
		_exit(2);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR){
  	printf("Signal handler failed\n");
  }

	printf("hello, world!\n");

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
		printf("a1_s is NULL for %s", line);
		return -1;
	}
	a2_s = strtok(NULL, DELIM);
	if (a2_s == NULL){
		printf("a2_s is NULL for %s", line);
		return -1;
	}
	table[lineno].clocks = (strtoull(nstime_s, NULL, 10) * COUNTS_PER_SECOND) / 1000000000L;
	table[lineno].pinP = strtol(pinP_s, NULL, 10);
	table[lineno].pinN = strtol(pinN_s, NULL, 10);
	table[lineno].a1 = strtol(a1_s, NULL, 10);
	table[lineno].a2 = strtol(a2_s, NULL, 10);
	printf("row: %lu time:%llu pinP:%i pinN:%i a1:%i\n", lineno, table[lineno].clocks, table[lineno].pinP, table[lineno].pinN, table[lineno].a1);
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
