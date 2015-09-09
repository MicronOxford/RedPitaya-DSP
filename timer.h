#include "xparameters.h"
#include <stdint.h>


typedef uint64_t XTime;

/************************** Constant Definitions *****************************/
#define GLOBAL_TMR_BASEADDR               (XPAR_GLOBAL_TMR_BASEADDR-0x200U)
#define GTIMER_COUNTER_LOWER_OFFSET       (0x00U+0x200U)
#define GTIMER_COUNTER_UPPER_OFFSET       (0x04U+0x200U)
#define GTIMER_CONTROL_OFFSET             (0x08U+0x200U)


/* Global Timer is always clocked at half of the CPU frequency */
#define COUNTS_PER_SECOND          (XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ /2)
/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

int initTimer();
void XTime_SetTime(XTime Xtime_Global);
void XTime_GetTime(XTime *Xtime_Global);
