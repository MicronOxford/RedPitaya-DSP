#include <stdint.h>

typedef struct actionTable {
    long double actionTime;
    int pinP;
    int pinN;
    uint32_t a1;
    uint32_t a2;
    double executedTime;
} actionTable_t;

actionTable_t *table;

long createActionTable(char *path);