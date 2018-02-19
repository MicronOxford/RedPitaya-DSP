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
#include "actionTable.h"

// #define MAXLINELEN 50
// #define MAXHANDLERLEN 5
// #define DELIM " "
// #define BILLION  1000000000L;

char ERRVAL[] = "/n";

uint32_t INT_MAX = UINT32_MAX;


void initializeAll();
int execActionTable(long lines);
void sig_handler(int signo);
void _exit(int status);

void spinwait(int loops) {
	int t = 0;
	while (t != loops)
		t += 1;
}

int main(int argc, char *argv[])
{
	printf("Initializing!\n");

	initializeAll();

	printf("Hello world!\n");

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
	actionTable = malloc(sizeof(actionLine)*lines);

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


void initializeAll() {
	actionTable = NULL;

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
		while (now  <= actionTable[line].clocks) XTime_GetTime(&now);
		out_setpins_P(actionTable[line].pinP);
		out_setpins_N(actionTable[line].pinN);
		fpga_awg_write_val_a(actionTable[line].a1);
		fpga_awg_write_val_b(actionTable[line].a2);
	}
	return 0;
}



void sig_handler(int signo){
  if (signo == SIGINT)
    _exit(5);
}

void _exit(int status) {
	free(actionTable);
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
