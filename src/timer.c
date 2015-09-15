#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <byteswap.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>

#include "timer.h"

#define PAGE_SIZE ((size_t)getpagesize())
#define PAGE_MASK ((uint64_t)(long)~(PAGE_SIZE - 1))


int TIMER_FD;
volatile uint8_t *TIMER_MMAP;

int initTimer(){
  TIMER_FD = open("/dev/mem", O_RDWR|O_SYNC);
  if (TIMER_FD < 0) {
      fprintf(stderr, "open(/dev/mem) failed (%d)\n", errno);
      return 1;
  }

  TIMER_MMAP = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, TIMER_FD, GLOBAL_TMR_BASEADDR);
  if (TIMER_MMAP == MAP_FAILED) {
      fprintf(stderr, "mmap64(0x%x@0x%x) failed (%d)\n",
              PAGE_SIZE, (uint32_t)(XPAR_GLOBAL_TMR_BASEADDR), errno);
      return 1;
  }
  return 0;
}

void XTime_SetTime(XTime Xtime_Global)
{
#ifdef USE_AMP
	if(*(volatile uint32_t *)(TIMER_MMAP+GTIMER_CONTROL_OFFSET) & (uint32_t)0x1){
		// Timer is already enabled so don't reset it
		return;
	}
#endif

	/* Disable Global Timer */
	*(volatile uint32_t *)(TIMER_MMAP+GTIMER_CONTROL_OFFSET) = 0x00;

	/* Updating Global Timer Counter Register */
	*(volatile uint32_t *)(TIMER_MMAP+GTIMER_COUNTER_LOWER_OFFSET) = (uint32_t)Xtime_Global;
	*(volatile uint32_t *)(TIMER_MMAP+GTIMER_COUNTER_UPPER_OFFSET) = (uint32_t)(Xtime_Global>>32U);

	/* Enable Global Timer */
	*(volatile uint32_t *)(TIMER_MMAP+GTIMER_CONTROL_OFFSET) = (uint32_t)0x1;
}

/****************************************************************************
*
* Get the time from the Global Timer Counter Register.
*
* @param	Pointer to the location to be updated with the time.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XTime_GetTime(XTime *Xtime_Global)
{
	uint32_t low;
	uint32_t high;

	/* Reading Global Timer Counter Register */
	do
	{
		high = *(volatile uint32_t *)(TIMER_MMAP+GTIMER_COUNTER_UPPER_OFFSET);
		low = *(volatile uint32_t *)(TIMER_MMAP+GTIMER_COUNTER_LOWER_OFFSET);
	} while(*(volatile uint32_t *)(TIMER_MMAP+GTIMER_COUNTER_UPPER_OFFSET) != high);

	*Xtime_Global = (((XTime) high) << 32U) | (XTime) low;
}
