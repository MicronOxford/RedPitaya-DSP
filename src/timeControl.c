/*
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

#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "timeControl.h"

#define PAGE_SIZE ((size_t)getpagesize())

volatile void *TIMER_MMAP;
volatile uint32_t *clock32;

/******************************************************************************/

int initTimer() {

    int memoryFileDescriptor = open("/dev/mem", O_RDWR|O_SYNC); // open /dev/mem
    if (memoryFileDescriptor < 0) {
        printf("can't open /dev/mem (errno %d) \n", errno);
        return 1;
    }

    TIMER_MMAP = mmap(
        NULL,
        PAGE_SIZE,
        PROT_READ|PROT_WRITE,
        MAP_SHARED,
        memoryFileDescriptor,
        TIMER_BASE_ADDRESS
    );

    close(memoryFileDescriptor);

    if (TIMER_MMAP == MAP_FAILED) {
        printf("mmap(0x%08x) failed (errno %d)\n", (uint32_t)TIMER_BASE_ADDRESS, errno);
        return 1;
    }

    currentTime = 0;
    clock32 = (volatile uint32_t *)&currentTime;

    return 0;
}

void updateCurrentTime() {
    do {
        clock32[1] = *(volatile uint32_t *)(TIMER_MMAP+TIMER_HIGH_OFFSET);
        clock32[0] = *(volatile uint32_t *)(TIMER_MMAP+TIMER_LOW_OFFSET);
    } while(*(volatile uint32_t *)(TIMER_MMAP+TIMER_HIGH_OFFSET) != clock32[1]);
}

uint64_t turnNSecToTicks(uint64_t nsec) {
    return nsec/NANOSEC_PER_TICK;
}