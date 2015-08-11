#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "rp.h"

#define MAXLINELEN 50
#define MAXHANDLERLEN 5
#define DELIM " "
#define BILLION  1000000000L;

/*
int ktime_get(void){
	return 0;
}

typedef int ktime_t;
*/

typedef struct actionTable {
	long long time;
	rp_dpin_t pin;
	long parameter;
	struct actionTable *next;
} actionTable_t;


int readActionTable(FILE *fp);
int readActionTableLine(char *line, actionTable_t *tableEntry);
int execActionTable();
void _exit(int status);

void spinwait(int loops) {
	int t = 0;
	while (t != loops)
		t += 1;
}

actionTable_t *table;

int main(int argc, char *argv[])
{

	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
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

	if (execActionTable() < 0) {
		printf("Failed to exec action table.\n");
		_exit(4);
	}
	printf("exec action table done.\n");

	rp_Release();
	exit(0);
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
		if (readActionTableLine(line, currRow) < 0)
			return -1;
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
	char *time_s;
	char *pin_s;
	char *parameter_s;

	time_s = strtok(line, DELIM);
	if (time_s == NULL){
		printf("action time is NULL for '%s'\n", line);
		return -1;
	}
	pin_s = strtok(NULL, DELIM);
	if (time_s == NULL){
		printf("pin is NULL for %s", line);
		return -1;
	}
	parameter_s = strtok(NULL, DELIM);
	if (time_s == NULL){
		printf("parameter_s is NULL for %s", line);
		return -1;
	}

	tableEntry->time = strtoll(time_s, NULL, 10);
	tableEntry->pin = (rp_dpin_t)strtol(pin_s, NULL, 10);
	tableEntry->parameter = strtol(parameter_s, NULL, 10);
	//printf("time:%lli pin:%i high:%i\n", tableEntry->time, tableEntry->pin, tableEntry->parameter);
	return 0;
}

/* need to add error handling on the clock */
int execActionTable() {
	struct timespec start, now;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	long int secs = table->time / BILLION;
	long int ns = table->time % BILLION;
	while (1) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &now);
		// could possibly loop in the ns part once we are in the right second, to reduce wait time/jitter?
		if (secs > now.tv_sec-start.tv_sec && ns > now.tv_nsec-start.tv_nsec){
			continue;
		}
		rp_DpinSetState(RP_DIO4_P, table->parameter);
		if (table->next != NULL) {
			table = table->next;
			secs = table->time / BILLION;
			ns = table->time % BILLION;
		} else {
			return 0;
		}
	}
}

void _exit(int status) {
	actionTable_t *next = table;
	while (next != NULL) {
		next = table->next;
		free(table);
		table = next;
	}
	exit(status);
}
