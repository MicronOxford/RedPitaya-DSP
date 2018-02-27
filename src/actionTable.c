#include <actionTable.h>
#include <stdio.h>
// #include <signal.h>
#include <string.h>
#include <stdlib.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <unistd.h>
// #include "timeControl.h"
// #include "gpioControl.h"
#include "timer.h"
#include "rpouts.h"


#define MAXLINELEN 50
#define DELIM " "
#define PRINT_READ_LINES 0


/*int readActionTable(FILE *fp, long lines);
int readActionTableLine(char *line, long lineNum);

long createActionTable(char *path) {

    FILE *fp;
    fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Could not open file %s\n", path);
        return -2;
    }

    long lines = 0;
    int ch;
    while (EOF != (ch=getc(fp))) {
        if (ch=='\n') { ++lines; }
    }
    printf("action table is %lu lines long\n", lines);
    rewind(fp);

    printf("alloc action table\n");
    table = malloc(sizeof(actionTable_t)*lines);

    if(readActionTable(fp, lines) != 0) {
        printf("Failed to read action table file.\n");
        return -3;
    }
    fclose(fp);
    printf("read action table file.\n");

    return lines;
}



int readActionTable(FILE *fp, long lines) {

    long lineNum;
    size_t lineLength = (size_t)MAXLINELEN;
    char *lineStr = (char *) malloc(sizeof(char)*(lineLength+1));

    for(lineNum = 0; lineNum < lines; lineNum++){
        if(getline(&lineStr, &lineLength, fp) <= 0){
            printf("read %lu lines of action table, but was empty\n", lineNum);
            free(lineStr);
            return -1;
        }
        if (readActionTableLine(lineStr, lineNum) < 0){
            printf("read ActionTableLine failed on %lu\n", lineNum);
            free(lineStr);
            return -1;
        }
    }
    free(lineStr);
    return 0;
}

int readActionTableLine(char *line, long lineNum) {

    char *nstime_s;
    char *pinP_s;
    char *pinN_s;
    char *a1_s;
    char *a2_s;

    if(PRINT_READ_LINES) {
        printf("processing line num %lu - '%s", lineNum, line);
    }

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
    //table[lineNum].clocks = (strtoull(nstime_s, NULL, 10) * COUNTS_PER_SECOND) / 1000000000L;
    table[lineNum].actionTime = strtold(nstime_s, NULL);
    table[lineNum].pinP = strtol(pinP_s, NULL, 10);
    table[lineNum].pinN = strtol(pinN_s, NULL, 10);
    table[lineNum].a1 = strtol(a1_s, NULL, 10);
    table[lineNum].a2 = strtol(a2_s, NULL, 10);
    if(PRINT_READ_LINES) {
        printf("row: %lu time:%Lf pinP:%i pinN:%i a1:%i a2:%i\n", lineNum, table[lineNum].actionTime, table[lineNum].pinP, table[lineNum].pinN, table[lineNum].a1, table[lineNum].a2);
    }
    return 0;
}*/

/******************************************************************************/

/*long createTestTable(long nanoInt, long numOfInt){

    printf("alloc test table\n");
    table = malloc(sizeof(testTable_t)*numOfInt);

    if(readActionTable(fp, lines) != 0) {
        printf("Failed to read action table file.\n");
        return -3;
    }
    printf("read action table file.\n");

    return lines;
}*/


/*int readActionTable(FILE *fp, long lines);
int readActionLine(char *line, long lineNum);

long int createActionTable(char *path) {

    FILE *fp;
    fp = fopen(path, "r"); //open file
    if (fp == NULL) {
        printf("Could not open file %s\n", path);
        return -2;
    }

    long int lines = 0;
    int ch;
    while (EOF != (ch=getc(fp))) { // counting lines
        if (ch=='\n') { ++lines; }
    }
    printf("action table is %lu lines long\n", lines);
    rewind(fp);

    printf("alloc action table\n");
    actionTable = malloc(sizeof(actionLine)*lines); // allocating memory

    if(readActionTable(fp, lines) != 0) {
        printf("Failed to read action table file.\n");
        return -3;
    }
    fclose(fp);
    printf("read action table file.\n");

    return lines;
}



int readActionTable(FILE *fp, long lines) {

    long lineNum;
    size_t lineLength = (size_t)MAXLINELEN;
    char *lineStr = (char *) malloc(sizeof(char)*(lineLength+1));

    for(lineNum = 0; lineNum < lines; lineNum++){
        if(getline(&lineStr, &lineLength, fp) <= 0){
            printf("read %lu lines of action table, but was empty\n", lineNum);
            free(lineStr);
            return -1;
        }
        if (readActionLine(lineStr, lineNum) < 0){
            printf("read ActionLine failed on %lu\n", lineNum);
            free(lineStr);
            return -1;
        }
    }
    free(lineStr);
    return 0;
}

int readActionLine(char *line, long lineNum) {

    char *nstime_s;
    char *pin_s;
    char *act_s;

    if(PRINT_READ_LINES) {
        printf("processing line num %lu - '%s", lineNum, line);
    }

    nstime_s = strtok(line, DELIM);  // REMINDER: first strtok call needs the string.
    if (nstime_s == NULL){
        printf("action nstime is NULL for '%s'\n", line);
        return -1;
    }
    pin_s = strtok(NULL, DELIM);
    if (pin_s == NULL){
        printf("pin is NULL for %s\n", line);
        return -1;
    }
    act_s = strtok(NULL, DELIM);
    if (act_s == NULL){
        printf("act is NULL for %s\n", line);
        return -1;
    }

    actionTable[lineNum].actionTime = turnNSecToTicks(strtoull(nstime_s, NULL, 10));
    actionTable[lineNum].pin = atoi(pin_s);
    actionTable[lineNum].action = atoi(act_s);
    setSignal(actionTable[lineNum].pin, actionTable[lineNum].action, &actionTable[lineNum].pinAddr, &actionTable[lineNum].valToWrit);
    actionTable[lineNum].executedTime = 0;

    if(PRINT_READ_LINES) {
        printf("row:%lu time:%llu pin:%i (@0x%p) action:%i (0x%08x)",
            lineNum, // row
            (unsigned long long int) actionTable[lineNum].actionTime, // time
            actionTable[lineNum].pin, actionTable[lineNum].pinAddr, // pinNum & pin address
            actionTable[lineNum].action, (unsigned int)actionTable[lineNum].valToWrit); // action value & value to write in the pin address (pinAddr)
    }
    return 0;
}*/




int readActionTableLine(char *line, long lineno);



long readActionTable(char *file) {


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
    if(actionTable == NULL) {
        printf(" Failed\n");
    }
    printf(" Completed\n");

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

    printf("ActionTable created successfully.\n");
    return lines;
}

int readActionTableLine(char *line, long lineno){

    char *nstime_s;
    // char *pinP_s;
    // char *pinN_s;
    char *pin_s;
    // char *a1_s;
    // char *a2_s;
    char *value_s;

    nstime_s = strtok(line, DELIM);  // REMINDER: first strtok call needs the str.
    if (nstime_s == NULL){
        printf("action nstime is NULL for '%s'\n", line);
        return -1;
    }
    // pinP_s = strtok(NULL, DELIM);
    // if (pinP_s == NULL){
    //     printf("pinP is NULL for %s\n", line);
    //     return -1;
    // }
    // pinN_s = strtok(NULL, DELIM);
    // if (pinN_s == NULL){
    //     printf("pinN is NULL for %s\n", line);
    //     return -1;
    // }
    pin_s = strtok(NULL, DELIM);
    if (pin_s == NULL){
        printf("pinN is NULL for %s\n", line);
        return -1;
    }
    // a1_s = strtok(NULL, DELIM);
    // if (a1_s == NULL){
    //     printf("a1_s is NULL for %s", line);
    //     return -1;
    // }
    // a2_s = strtok(NULL, DELIM);
    // if (a2_s == NULL){
    //     printf("a2_s is NULL for %s", line);
    //     return -1;
    // }
    value_s = strtok(NULL, DELIM);
    if (value_s == NULL){
        printf("pinN is NULL for %s\n", line);
        return -1;
    }
    //actionTable[lineno].clocks = (strtoull(nstime_s, NULL, 10) * COUNTS_PER_SECOND) / 1000000000L;
    actionTable[lineno].nsec = strtoull(nstime_s, NULL, 10);
    // actionTable[lineno].pinP = strtol(pinP_s, NULL, 10);
    // actionTable[lineno].pinN = strtol(pinN_s, NULL, 10);
    actionTable[lineno].pin = atoi(pin_s);
    // actionTable[lineno].a1 = strtol(a1_s, NULL, 10);
    // actionTable[lineno].a2 = strtol(a2_s, NULL, 10);
    actionTable[lineno].value = strtol(value_s, NULL, 10);
    // printf("row: %lu time:%llu pinP:%i pinN:%i a1:%i\n", lineno, actionTable[lineno].clocks, actionTable[lineno].pinP, actionTable[lineno].pinN, actionTable[lineno].a1);
    printf("row: %lu time:%llu pin:%i value:%i\n",
        lineno,
        (unsigned long long int) actionTable[lineno].nsec,
        actionTable[lineno].pin,
        actionTable[lineno].value);

    actionTable[lineno].clocks = turnNSecToTicks(actionTable[lineno].nsec);
    return setPinVal(actionTable[lineno].pin, actionTable[lineno].value,
        &actionTable[lineno].pinAddr, &actionTable[lineno].valToWrite);
}
