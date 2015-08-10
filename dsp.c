#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

#define MAXLINELEN 50
#define MAXHANDLERLEN 5
#define DELIM " "

int ktime_get(void){
	return 0;
}

typedef int ktime_t;

typedef struct actionTable {
	int time;
	rp_dpin_t pin;
	int parameter;
	struct actionTable *next;
} actionTable_t;


int readActionTable(FILE *fp);
int readActionTableLine(char *line);
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

	if (execActionTable() < 0) {
		printf("Failed to exec action table.\n");
		_exit(4);
	}

	rp_Release();
	exit(0);
}

/******************************************************************************/

int readActionTable(FILE *fp) {
	int bytes_read = 0;
	size_t linelen = (size_t)MAXLINELEN;
	char *line;
	line = (char *) malloc(sizeof(char)*(linelen+1));

	while (1) {
		getline(&line, &linelen, fp);
		if (bytes_read <= 0)
			return 0;
		if (readActionTableLine(line) < 0)
			return 0;
		table->next = (actionTable_t *) malloc(sizeof(actionTable_t));
	}
}

int readActionTableLine(char *line){
	char *time_s;
	char *pin_s;
	char *parameter_s;

	time_s = strtok(line, DELIM);
	if (time_s == NULL){
		printf("action time is NULL for %s", line);
		return -1;
	}
	pin_s = strtok(line, DELIM);
	if (time_s == NULL){
		printf("pin is NULL for %s", line);
		return -1;
	}
	parameter_s = strtok(line, DELIM);
	if (time_s == NULL){
		printf("parameter_s is NULL for %s", line);
		return -1;
	}

	table->time = strtol(time_s, NULL, 10);
	table->pin = strtol(pin_s, NULL, 10);
	table->parameter = strtol(parameter_s, NULL, 10);
	return 0;
}

int execActionTable() {
	ktime_t start = ktime_get();
	while (1) {
		if (table->time > start-ktime_get())
			continue;
		rp_DpinSetState(table->pin, table->parameter);
		if (table->next != NULL) {
			table = table->next;
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
