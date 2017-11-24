//#include <time.h>
#include <stdint.h>

#define BILLION  1000000000
#define ARM_QA7_CONTROL_REG 0x40000000
#define ARM_TIMER_OFFSET 7
//#define CLOCK_LSB 7
//#define CLOCK_MSB 8
#define PROC_FREQ 19200000 //19.2MHz
//#define NANO_PER_CLICK 52


double getNSecPerTick();
uint64_t turnNSecToTicks(unsigned long long int nSec);

int initARMTimer();
void startARMTimer();
//void resetTestTime();
//void updateARMTimer();
void getARMTimer(uint64_t *utime);
void setNextTime(uint64_t nTime);

int isARMTimerLessThanNext();
void waitForNext();

void printARMControl();
void printARMTime();