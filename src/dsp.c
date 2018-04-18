/*
* Copyright (C) 2015 Tom Parks
* Copyright (C) 2017-2018 Tiago Susano Pinto <tiagosusanopinto@gmail.com>
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

#include "timeControl.h"
#include "rpouts.h"
#include "actionTable.h"

void initializeAll();
void setMaxPriority();
int execActionTable(const long lines);
void sig_handler(int signo);
void _exit(int status);

/******************************************************************************/

int main(int argc, char *argv[]) {
    printf("Hello world!\n");

    if (argc != 2) {
        printf("num of args used was %u\n", argc);
        printf("USAGE: dsp \"action-table-file\"\n");
        _exit(1);
    }

    initializeAll();

    long lines = createActionTable(argv[1]);
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
int execActionTable(const long lines) {
    printf("executing ActionTable\n");
    long line;

    printf("set time\n");
    uint64_t nextTime = 0;
    updateCurrentTime();
    uint64_t startTime = currentTime;

    for (line = 0; line < lines; line++){
        actionLine actLine = actionTable[line];
        nextTime = startTime + actLine.clocks;

        if (actLine.action < 0) {
            int pinNum = actLine.pin;
            int inputType = abs(actLine.action);
            if (pinNum < 0 || pinNum > 15 || inputType > 3) {
                printf("Error not expected at line %ld\n", line);
            }

            volatile uint32_t * memAddr = getPinPDir();
            if (pinNum > 7) {
                pinNum -= 8;
                memAddr = getPinNDir();
            }

            while (currentTime < nextTime) updateCurrentTime();

            *memAddr &= ~(1 << pinNum);

            if(inputType == 3) {
                if ((*(actLine.actAddr) & actLine.actVal)) {
                    inputType = 2;
                } else {
                    inputType = 1;
                }
            }

            if (inputType == 1) {
                while ((*(actLine.actAddr) & actLine.actVal) == 0) { }
            } else {
                while ((*(actLine.actAddr) & actLine.actVal) != 0) { }
            }

            updateCurrentTime();
            startTime = currentTime;
            *memAddr |= (1 << pinNum);
        } else {
            while (currentTime < nextTime) updateCurrentTime();
            *(actLine.actAddr) = actLine.actVal;
        }
    }
    
    return 0;
}

/******************************************************************************/

void sig_handler(int signo) {
    if (signo == SIGINT) {
        _exit(5);
    }
}

void _exit(int status) {
    free(actionTable);
    exitOuts();
    exit(status);
}
