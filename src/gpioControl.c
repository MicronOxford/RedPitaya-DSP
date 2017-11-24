#include "gpioControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

//#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
#define MAX_PIN 54

// I/O access
volatile unsigned *gpio;

// volatile uint32_t *toSendAddr;
// uint32_t toSendValue;

char pinsFunc[MAX_PIN];

int sign9 = 1; //for signalChg9 function


int initGPIO() {
    int  mem_fd = open("/dev/mem", O_RDWR|O_SYNC); // open /dev/mem
    if (mem_fd < 0) {
        printf("can't open /dev/mem \n");
        return 1;
    }

    /* mmap GPIO */
    gpio = mmap(
        NULL,             //Any adddress in our space will do
        BLOCK_SIZE,       //Map length
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        GPIO_BASE         //Offset to GPIO peripheral
        );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (gpio == MAP_FAILED) {
        printf("mmap error 0x%08x\n", (uint32_t)GPIO_BASE);//errno also set!
        return 1;
    }


    // prinfSTUFF();
    //For testing, GPIO pin number 9 is going to be used

    //Before using pin as output mode, must be config to input mode
    //Changing the mode of pin 9 is in the address +0 (the values stored is 32bits)
    //each 3 bits correspond to a pin (the last 2 bits are reseverd)

    //Init intput in GPIO pin number 9
    *(gpio+GPIO_FUNCTION_SEL_0) &= ~(7<<(9*3)); // change the bits 27-29 to input mode (000)

    // prinfSTUFF();

    //Init output in GPIO pin number 9
    *(gpio+GPIO_FUNCTION_SEL_0) |= (1<<(9*3)); // change the bits 27-29 to output mode (001)

    // prinfSTUFF();

    return 0;
}

void initPinFunct() {
    int i=0;
    for(i=0; i<MAX_PIN; i++) {
        if(pinsFunc[i] != 0) {
            int addrOffset = GPIO_FUNCTION_SEL_0 + (i/10);
            int valOffset = (i%10)*3;
            *(gpio+addrOffset) &= ~(7<<valOffset);
            if(pinsFunc[i] == 'O') {
                *(gpio+addrOffset) |= (1<<valOffset);
            }
        }
    }
}

void signal9(int i) { //send signal in pin 9 if i != 0, else clear it
    if(i != 0) {
        //To set a signal to pin 9, must change the 9th bit to 1 in the address +7
        *(gpio+GPIO_OUTPUT_SET_0) = (1<<9);
    } else {
        //To clear a signal in pin 9, must change the 9th bit to 1 in the address +10
        *(gpio+GPIO_OUTPUT_CLEAR_0) = (1<<9);
    }
}

void signalON9() {
    *(gpio+GPIO_OUTPUT_SET_0) = (1<<9);
}

void signalOFF9() {
    *(gpio+GPIO_OUTPUT_CLEAR_0) = (1<<9);
}

void signalONAll() {
    *(gpio+GPIO_OUTPUT_SET_0) = 0xffffffff;
}

void signalOFFAll() {
    *(gpio+GPIO_OUTPUT_CLEAR_0) = 0xffffffff;
}

void signalChg9() {
    if(sign9 != 0) {
        //To set a signal to pin 9, must change the 9th bit to 1 in the address +7
        *(gpio+GPIO_OUTPUT_SET_0) = (1<<9);
        sign9 = 0;
    } else {
        //To clear a signal in pin 9, must change the 9th bit to 1 in the address +10
        *(gpio+GPIO_OUTPUT_CLEAR_0) = (1<<9);
        sign9 = 1;
    }
}


void setSignal9(int i, volatile uint32_t **addr, uint32_t *val) {
    if(i != 0) {
        *addr = (gpio+GPIO_OUTPUT_SET_0);
        *val = (1<<9);
    } else {
        *addr = (gpio+GPIO_OUTPUT_CLEAR_0);
        *val = (1<<9);
    }
}

void setSignal(int pin, int act, volatile uint32_t **addr, uint32_t *val) {

    int GPIOoffset = 0;
    
    if(act < 0){
        pinsFunc[pin] = 'I'; //Input
        GPIOoffset = GPIO_LEVEL_0;
    } else {
        pinsFunc[pin] = '0'; //Output
        if(act != 0) {
            GPIOoffset = GPIO_OUTPUT_SET_0;
        } else {
            GPIOoffset = GPIO_OUTPUT_CLEAR_0;
        }
    }

    if(pin > 31) {
        pin -= 32;
        GPIOoffset++;
    }
    *addr = (gpio+GPIOoffset);
    *val = (1<<pin);
}

/*void setToSend(uint32_t *addr, uint32_t val) {
    toSendAddr = addr;
    toSendValue = val;
}

inline void useToSend() {
    *(toSendAddr) = toSendValue;
}*/

void prinfSTUFF() {
    printf("Printing functionSel outputset outputclear\n");
    printFuncSel(); printOutSet(); printOutClr();
}

void printFuncSel() {
    int val = *(gpio+GPIO_FUNCTION_SEL_0);
    printf("%x\n", val);
}

void printOutSet() {
    int val = *(gpio+GPIO_OUTPUT_SET_0);
    printf("%x\n", val);
}

void printOutClr() {
    int val = *(gpio+GPIO_OUTPUT_CLEAR_0);
    printf("%x\n", val);
}