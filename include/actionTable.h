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
//    unsigned long long clocks;
    uint64_t nsec;
//    int pinP;
//    int pinN;
    int pin;
//    uint32_t a1;
//    uint32_t a2;
    uint32_t value;
    volatile uint32_t * pinAddr;
    uint32_t valToWrit;
} actionLine;

actionLine *actionTable;

long readActionTable(char *file);
// int readActionTableLine(char *line, long lineno);