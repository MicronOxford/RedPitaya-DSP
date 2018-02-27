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
void setMaxPriority();
int execActionTable(long lines);
void sig_handler(int signo);
void _exit(int status);

/*void spinwait(int loops) {
	int t = 0;
	while (t != loops)
		t += 1;
}*/

int main(int argc, char *argv[])
{
	printf("Hello world!\n");

	if (argc != 2) {
        printf("num of args used was %u\n", argc);
		printf("USAGE: dsp \"action-table-file\"\n");
		_exit(1);
	}

	initializeAll();

	long lines = readActionTable(argv[1]);
	if(lines < 0) {
		printf("Failed to read/create action table.\n");
		_exit(abs(lines));
	}

	// before we set ourselves to more important than the terminal, flush.
	fflush(stdout);

	setMaxPriority();

	int execstatus = execActionTable(lines);
	if (execstatus < 0) {
		printf("Failed to execute ActionTable (%i).\n", execstatus);
		_exit(5);
	}
	printf("ActionTable executed.\n");

	_exit(0);
}

/******************************************************************************/


void initializeAll() {
	printf("Initializing!\n");

	actionTable = NULL;

	if (initTimer() == 1){
		printf("init timer failed\n");
		_exit(2);
	}

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


void setMaxPriority() {
	struct sched_param params;
	params.sched_priority = 99;
	if (sched_setscheduler(0, SCHED_FIFO, &params) == -1){
		printf("Failed to set priority.\n");
		_exit(4);
	}
}

/* need to add error handling on the clock */
int execActionTable(long lines) {
	printf("executing ActionTable\n");
	long line;
	// float a_volts = 0;
	// rp_GenWaveform(RP_CH_1, RP_WAVEFORM_DC);
	// rp_GenWaveform(RP_CH_2, RP_WAVEFORM_DC);
	// rp_GenAmp(RP_CH_1, a_volts);
	// rp_GenAmp(RP_CH_2, a_volts);
	// rp_GenOutEnable(RP_CH_1);
	// rp_GenOutEnable(RP_CH_2);

	// printf("faffing with actiontables\n");
	// XTime now;

	printf("set time\n");
	uint64_t nextTime;
	updateCurrentTime();
	updateStartTime(currentTime);
	// XTime_SetTime(0);
	// XTime_GetTime(&now);

	for (line = 0; line < lines; line++){
		nextTime = startTime + actionTable[line].nsec;

		while (currentTime  < nextTime) updateCurrentTime();

		*(actionTable[line].pinAddr) = actionTable[line].valToWrite;
	}
	/*int analogVal = 0;
	char letter[10];
	fpga_awg_write_val_a(analogVal);
	while(1) {
		printf("current analogVal %d\n", analogVal);
		scanf("%s", letter);
		if(letter[0] == 'q'){
			analogVal += 1;
			continue;
		}
		if(letter[0] == 'w'){
			analogVal -= 1;
			continue;
		}
		if(letter[0] == 'a'){
			analogVal += 10;
			continue;
		}
		if(letter[0] == 's'){
			analogVal -= 10;
			continue;
		}
		if(letter[0] == 'z'){
			analogVal += 100;
			continue;
		}
		if(letter[0] == 'x'){
			analogVal -= 100;
			continue;
		}
		if(letter[0] == 'm'){
			break;
		}
	}*/
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
