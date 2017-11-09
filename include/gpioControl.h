#include <stdint.h>

#define BCM2708_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x00200000) /* GPIO controller */
#define GPIO_FUNCTION_SEL_0      0
#define GPIO_OUTPUT_SET_0        7
#define GPIO_OUTPUT_CLEAR_0      10


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
// #define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
// #define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
// #define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
 
// #define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
// #define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
 
// #define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
 
// #define GPIO_PULL *(gpio+37) // Pull up/pull down
// #define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock


int initGPIO();
void signal9(int i);
void signalON9();
void signalOFF9();
void signalONAll();
void signalOFFAll();
void signalChg9();

void setSignal9(int i, volatile uint32_t **addr, uint32_t *val);
//void setToSend(uint32_t *addr, uint32_t val);
//inline void useToSend();


void prinfSTUFF();
void printFuncSel();
void printOutSet();
void printOutClr();
