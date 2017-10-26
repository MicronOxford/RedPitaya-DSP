#include <actionTable.h>
#include <stdio.h>
// #include <stdio.h>
// #include <signal.h>
#include <string.h>
#include <stdlib.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <unistd.h>

#define MAXLINELEN 50
#define DELIM " "
#define PRINT_READ_LINES 0


int readActionTable(FILE *fp, long lines);
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
}

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