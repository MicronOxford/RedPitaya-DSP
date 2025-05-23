/*
* Copyright (C) 2015 Tom Parks <thomasparks@outlook.com>
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

#include <actionTable.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rpouts.h"
#include "timeControl.h"

#define MAXLINELEN 50
#define DELIM " "
#define PRINT_LINES 0

int readActionTableLine(char *line, long lineno);

/******************************************************************************/

long createActionTable(char *file) {
    FILE *fp;
    fp = fopen(file, "r");
    if (fp == NULL) {
        printf("Could not open file %s\n", file);
        return -2;
    }

    long lines = 0;
    int ch;
    while (EOF != (ch=getc(fp))) {
        if (ch=='\n') ++lines;
    }
    printf("ActionTable is %lu lines long\n", lines);
    rewind(fp);

    printf("Memory allocation for ActionTable...");
    actionTable = malloc(sizeof(actionLine)*lines);
    if (actionTable == NULL) {
        printf(" Failed\n");
        return -1;
    }
    printf(" Completed\n");

    int bytes_read = 0;
    long lineno = 0;
    size_t linelen = (size_t)MAXLINELEN;
    char *linestr;
    linestr = (char *) malloc(sizeof(char)*(linelen+1));

    for (lineno = 0; lineno < lines; lineno++) {
        bytes_read = getline(&linestr, &linelen, fp);
        if (bytes_read <= 0) {
            printf("ERROR - line %lu of action table was empty\n", lineno);
            free(linestr);
            return -1;
        }

        int readActionLineVal = readActionTableLine(linestr, lineno);
        if (readActionLineVal < 0) {
            printf("readActionTableLine failed on line number %lu\n", lineno);
            free(linestr);
            return -1;
        }
        if (readActionLineVal == 1) {
            printf("Previous warning(s) were on line number %lu\n", lineno);
        }
    }
    free(linestr);

    printf("ActionTable created successfully.\n");
    return lines;
}

int readActionTableLine(char *line, long lineno){

    char *nstime_s;
    char *pin_s;
    char *action_s;

    nstime_s = strtok(line, DELIM);  // REMINDER: first strtok call needs the str.
    if (nstime_s == NULL){
        printf("nstime is NULL for '%s'\n", line);
        return -1;
    }
    pin_s = strtok(NULL, DELIM);
    if (pin_s == NULL){
        printf("pin number is NULL for %s\n", line);
        return -1;
    }
    action_s = strtok(NULL, DELIM);
    if (action_s == NULL){
        printf("action is NULL for %s\n", line);
        return -1;
    }

    actionTable[lineno].nsec = strtoull(nstime_s, NULL, 10);
    actionTable[lineno].pin = atoi(pin_s);
    actionTable[lineno].action = atoi(action_s);

    if(PRINT_LINES) {
        printf("Line Number %lu time:%llu pin:%i action:%i\n",
            lineno,
            (unsigned long long int) actionTable[lineno].nsec,
            actionTable[lineno].pin,
            actionTable[lineno].action);
    }

    actionTable[lineno].clocks = turnNSecToTicks(actionTable[lineno].nsec);
    return setPinVal(actionTable[lineno].pin, &actionTable[lineno].action,
        &actionTable[lineno].actAddr, &actionTable[lineno].actVal);
}
