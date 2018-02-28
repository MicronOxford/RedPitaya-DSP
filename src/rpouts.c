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
#include "fpga_awg.h"

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
              (long int)PAGE_SIZE, (uint32_t)(RP_BASE_ADDR), errno);
      return 1;
  }

// turn all pins into ouput mode
  *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_DIR) |= 0x000000FF;
  *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_DIR) |= 0x000000FF;
//set all pins to 0
  *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT) = 0;
  *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_OUT) = 0;
//set all pins' starting state to 0
  pinsP = 0;
  pinsN = 0;

  return 0;
}

inline void out_setpins_P(int pins){
  *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT) = pins;
}

inline void out_setpins_N(int pins){
  *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_OUT) = pins;
}

void turnLEDs(int leds) {
  *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+LED) = leds;
}

/*void setPinToInput(int pin) {
  volatile uint32_t * memAddr;
  if(pin < 8) {
    memAddr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_DIR);
  } else {
    memAddr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_DIR);
    pin -= 8;
  }
  *memAddr &= ~(1<<pin);
}*/

/*void setPinToOutput(int pin) {
  volatile uint32_t * memAddr;
  if(pin < 8) {
    memAddr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_DIR);
  } else {
    memAddr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_DIR);
    pin -= 8;
  }
  *memAddr |= (1<<pin);
}*/

/*uint32_t out_getpins(){
  return *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT);
}*/

/*
pin [0-7]   =   pinP [0-7]
pin [8-15]  =   pinN [0-7]
pin -1 & -2 =   analogue output 1 & 2
*/
int setPinVal(int pin, int action, volatile uint32_t ** addr, uint32_t *val) {
  if(pin < 0) {
    // analogue output
    // pin -1 = OUT1
    // pin -2 = OUT2
    // action [-4096,-4095] = output [-1,1]V
    pin  = abs(pin);
    if(pin > 2) {
      printf("Analogue pin number was bigger than 2 (%d)\n", pin);
      return -1;
    }

    *addr = get_awg_chanel_mem(pin);
    *val = action; //TODO check range of values

    if(*addr == NULL) {
      printf("Error processing analogue output\n");
      return -1;
    }
  } else {
    // digital pin out/in put (in E1)
    // pins[0-7]  = DIO[0-7]_P
    // pins[8-15] = DIO[0-7]_N
    if(pin > 15) {
      printf("Digital pin number was bigger than 15 (%d)\n", pin);
      return -1;
    }

    if(action < 0) {
      // digital input
      // action -1 = wait for signal to be 1
      // action -2 = wait for signal to be 0
      // action -3 = wait for edge
      if(action < -3) {
        printf("WARNING! Action was %i - digital input action should be -1, -2 or -3 (-3 apply by default)!\n", action);
        action = -3;
      }

      if(pin > 7) {
        pin -= 8;
        *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_IN);
      } else {
        *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_IN);
      }
      *val = (1<<pin);

    } else {
      // digital output
      // action 0 = clear pin   (stop signal)
      // action 1 = set pin     (send signal)
      if(action > 1){
        printf("WARNING! Action was %i - digital output action should be 1 or 0 (1 apply by default)\n", action);
        action = 1;
      }

      volatile uint32_t *pinStates;
      if(pin > 7) {
        pin -= 8;
        *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_OUT);
        pinStates = &pinsN;
      } else {
        *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT);
        pinStates = &pinsP;
      }

      if(action) {
        *pinStates |= (1<<pin); 
      } else {
        *pinStates &= !(1<<pin);
      }
      *val = *pinStates;
    }
  }
  return 0;
}

volatile uint32_t * getPinPDir() {
  return (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_IN);
}

volatile uint32_t * getPinNDir() {
  return (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_IN);
}