/*
* Copyright (C) 2015 Tom Parks <thomasparks@outlook.com>
* Copyright (C) 2017-2018 Tiago Susano Pinto <tiagosusanopinto@gmail.com>
*
* This file is part of RedPitaya-DSP.
*
* RedPitaya-DSP is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* RedPitaya-DSP is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with RedPitaya-DSP. If not, see <http://www.gnu.org/licenses/>.
*/

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>

#include "rpouts.h"
#include "RPmemmap.h"

#define PAGE_SIZE ((size_t)getpagesize())
#define ASG_BASE_SCALE 0x2000

volatile uint8_t *OUTS_MMAP; // fpga memory mapping address
uint32_t pinsP; // pins P state
uint32_t pinsN; // pins N state

/******************************************************************************/

int initOuts() {
    int memoryFileDescriptor = open("/dev/mem", O_RDWR|O_SYNC);
    if (memoryFileDescriptor < 0) {
        fprintf(stderr, "open(/dev/mem) failed (%d)\n", errno);
        return 1;
    }

    OUTS_MMAP = mmap(NULL,
        PAGE_SIZE*2048,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        memoryFileDescriptor,
        RP_BASE_ADDR);

    close(memoryFileDescriptor);

    if (OUTS_MMAP == MAP_FAILED) {
        fprintf(stderr, "mmap64(0x%lx@0x%x) failed (%d)\n",
            (long int)PAGE_SIZE, (uint32_t)(RP_BASE_ADDR), errno);
        return 1;
    }

    // turn all pins into ouput mode
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+DIRECT_PINP) |= 0x000000FF;
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+DIRECT_PINN) |= 0x000000FF;
    // set all pins to 0
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+OUT_PINP) = 0;
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+OUT_PINN) = 0;
    // set all pins' starting state to 0
    pinsP = 0;
    pinsN = 0;
    // set to 0 (turn off) all LEDs
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+LED) = 0;

    // reset ASG to send continous signal
    *(volatile uint32_t *)(OUTS_MMAP+ASG_OFFSET+ASG_CONFIG) = ((1<<ASG_CONFIG_SMRESET_CHA) | (1<<ASG_CONFIG_SMRESET_CHB)) & ASG_CONFIG_NOTRESERVED;
    *(volatile uint32_t *)(OUTS_MMAP+ASG_OFFSET+SCALEOFF_CHA) = ((0x0 << SCALEOFF_OFFSET) | (ASG_BASE_SCALE << SCALEOFF_SCALE)) & SCALEOFF_NOTRESERVED;
    *(volatile uint32_t *)(OUTS_MMAP+ASG_OFFSET+SCALEOFF_CHB) = ((0x0 << SCALEOFF_OFFSET) | (ASG_BASE_SCALE << SCALEOFF_SCALE)) & SCALEOFF_NOTRESERVED;

    // set analog output signal to 0
    *(volatile uint32_t *)(OUTS_MMAP+ASG_OFFSET+OUTPUT_CHA) = 0;
    *(volatile uint32_t *)(OUTS_MMAP+ASG_OFFSET+OUTPUT_CHB) = 0;

    return 0;
}

int exitOuts() {
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+OUT_PINP) = 0;
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+OUT_PINN) = 0;
    // *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+LED) = 0;

    *(volatile uint32_t *)(OUTS_MMAP+ASG_OFFSET+OUTPUT_CHA) = 0;
    *(volatile uint32_t *)(OUTS_MMAP+ASG_OFFSET+OUTPUT_CHB) = 0;

    // return munmap(OUTS_MMAP, PAGE_SIZE*2048); //TODO add print about error
    return 0;
}

void setPinsP(uint32_t pins) {
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+OUT_PINP) = pins;
}

void setPinsN(uint32_t pins) {
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+OUT_PINN) = pins;
}

void setLEDs(uint32_t leds) {
    *(volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+LED) = leds;
}

/*
* pin -1 & -2 = analog output 1 & 2
* pin [0-7]   = pinP [0-7]
* pin [8-15]  = pinN [0-7]
*/
int setPinVal(int pin, int *action, volatile uint32_t ** addr, uint32_t *val) {
    int returnVal = 0;
    if (pin < 0) {
        // analog output
        // pin -1 = OUT1
        // pin -2 = OUT2
        // action [-8192,8191] = output [-1,1]V
        pin  = abs(pin);

        switch (pin) {
            case 1:
                *addr = (volatile uint32_t *)(OUTS_MMAP+ASG_OFFSET+OUTPUT_CHA);
                break;
            case 2:
                *addr = (volatile uint32_t *)(OUTS_MMAP+ASG_OFFSET+OUTPUT_CHB);
                break;
            default:
                printf("Analog pin number was bigger than 2 (%d)\n", pin);
                return -1;
        }

        if (*action < 0) {
            if (*action < -8192) { // -0x2000
                printf("WARNING! Action was %i. Negative analog output should be between -1 to -8192 (-8192 apply by default)\n", *action);
                *action = -8192;
                returnVal = 1;
            }
            *action += 0x4000;
        } else {
            if (*action > 8191) { // 0x1FFF
                printf("WARNING! Action was %i. Positive analog output should be between 0 to 8191 (8191 apply by default)\n", *action);
                *action = 8191;
                returnVal = 1;
            }
        }
        *val = *action;

        if (*addr == NULL) {
            printf("Error processing analog output\n");
            return -1;
        }
    } else {
        // digital pin out/in put (in E1)
        // pins[0-7]  = DIO[0-7]_P
        // pins[8-15] = DIO[0-7]_N
        if (pin > 15) {
            printf("Digital pin number was bigger than 15 (%d)\n", pin);
            return -1;
        }

        if (*action < 0) {
            // digital input
            // action -1 = wait for signal to be 1
            // action -2 = wait for signal to be 0
            // action -3 = wait for edge
            if (*action < -3) {
                printf("WARNING! Action was %i. Digital input action should be -1, -2 or -3 (-3 apply by default)\n", *action);
                *action = -3;
                returnVal = 1;
            }

            if (pin < 8) {
                *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+IN_PINP);
            } else {
                pin -= 8;
                *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+IN_PINN);
            }
            *val = (1<<pin);

        } else {
            // digital output
            // action 0 = clear pin   (stop signal)
            // action 1 = set pin     (send signal)
            if (*action > 1) {
                printf("WARNING! Action was %i. Digital output action should be 1 or 0 (1 apply by default)\n", *action);
                *action = 1;
                returnVal = 1;
            }

            volatile uint32_t *pinStates;
            if (pin < 8) {
                *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+OUT_PINP);
                pinStates = &pinsP;
            } else {
                pin -= 8;
                *addr = (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+OUT_PINN);
                pinStates = &pinsN;
            }

            if (*action) {
                *pinStates |= (1<<pin);
            } else {
                *pinStates &= ~(1<<pin);
            }
            *val = *pinStates;
        }
    }
    return returnVal;
}

volatile uint32_t * getPinPDir() {
    return (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+IN_PINP);
}

volatile uint32_t * getPinNDir() {
    return (volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+IN_PINN);
}
