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

#include <stdint.h>
// #include <stdio.h>

/*typedef struct actionTable {
    long double actionTime;
    int pinP;
    int pinN;
    uint32_t a1;
    uint32_t a2;
    double executedTime;
} actionTable_t;*/

/*typedef struct { // struct ACTION LINE
    uint64_t actionTime; // action time (in clock ticks)
    int pin; // pin number
    int action; // action (1: set pin on; 0: clear pin off; -1: wait for pin)
    volatile uint32_t * pinAddr; // pin's virtual address to write the action (given by gpioControl)
    uint32_t valToWrit; // value to write in order to execute the action on the pin (given by gpioControl)
    unsigned long long int executedTime; // ~time when the action was executed (CURRENTLY NOT IMPLEMENT)
} actionLine;

//actionTable_t *table;
actionLine *actionTable; //
long int numberOfLines;

//long createActionTable(char *path);
long createActionTable(char *path);*/



typedef struct actionTable {
    uint64_t nsec;
    int pin;
    int action;
    uint64_t clocks;
    volatile uint32_t * pinAddr;
    uint32_t valToWrite;
} actionLine;

actionLine *actionTable;

long readActionTable(char *file);
// int readActionTableLine(char *line, long lineno);