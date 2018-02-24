//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
//#include <byteswap.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>

#include "rpouts.h"
#include "RPmemmap.h"

#define PAGE_SIZE ((size_t)getpagesize())
#define PAGE_MASK ((uint64_t)(long)~(PAGE_SIZE - 1))

volatile uint8_t *OUTS_MMAP; // pins memory address
volatile uint32_t pinsP;  // pins P state
volatile uint32_t pinsN;  // pins N state

int initOuts(){
  int OUTS_FD = open("/dev/mem", O_RDWR|O_SYNC);
  if (OUTS_FD < 0) {
      fprintf(stderr, "open(/dev/mem) failed (%d)\n", errno);
      return 1;
  }

  // map the whole of the RP FPGA space
  OUTS_MMAP = mmap(NULL,
    PAGE_SIZE*2048,
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    OUTS_FD,
    RP_BASE_ADDR);
  if (OUTS_MMAP == MAP_FAILED) {
      fprintf(stderr, "mmap64(0x%lx@0x%x) failed (%d)\n",
              PAGE_SIZE, (uint32_t)(RP_BASE_ADDR), errno);
      return 1;
  }

// turn all pins into ouput mode
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_DIR) |= 0x000000FF;
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_DIR) |= 0x000000FF;
//set all pins to 0
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT) = 0;
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_OUT) = 0;
//set all pins' starting state to 0
  pinsP = 0;
  pinsN = 0;

  return 0;
}

inline void out_setpins_P(int pins){
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT) = pins;
}

inline void out_setpins_N(int pins){
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_OUT) = pins;
}

void turnLEDs(int leds) {
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+LED) = leds;
}

/*uint32_t out_getpins(){
  return *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT);
}*/

/*
pin [0-7]   =   pinP [0-7]
pin [8-15]  =   pinN [0-7]
pin [16-17] =   analogue output [1-2]
*/
int setPinVal(int pin, int value, volatile uint32_t ** addr, uint32_t *val) {
  if(pin < 0 || pin >17) {
    //print error
    return -1;
  }
  if(pin > 15) {
    // analogue output - NOT IMPLEMENTED YET
    return 1;
  }
  if(pin > 7) {
    pin -= 8;
    *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_OUT);
    if(value) {
      pinsN |= (1<<pin); 
    } else {
      pinsN &= !(1<<pin);
    }
    *val = pinsN;
  } else {
    *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT);
    if(value) {
      pinsP |= (1<<pin); 
    } else {
      pinsP &= !(1<<pin);
    }
    *val = pinsP;
  }
  return 0;
}