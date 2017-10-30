#include <time.h>
#define PRINT_RESOLUTION 0
#define BILLION  1000000000

#define ARM_QA7_CONTROL_REG 0x40000000
#define CLOCK_LSB 7
#define CLOCK_MSB 8


//#define PBASE 0x3F000000
//#define ARM_TIMER_CTL   (PBASE+0x0000B408)
//#define ARM_TIMER_CNT (PBASE+0x0000B420)

#define NANO_PER_CLICK 52

void startTime();
double getTimeSinceStart();
int compareNextToNow();
void addNanoSecToNext(long nsec);
long getSecondsSinceStart();

void printTimeControl();
int compareTimeToNow(struct timespec *time);
void addNanoSecToTime(long nsec, struct timespec *time);
struct timespec getNow();
double turnTime(struct timespec time);



int initARMTimer();
//void resetTestTime();
void updateARMTimer();
void getARMTimer(unsigned long int *lsbTime, unsigned long int *msbTime);
void setNextTime(unsigned long int lsbTime, unsigned long int msbTime);
int isARMTimerLessThanNext();