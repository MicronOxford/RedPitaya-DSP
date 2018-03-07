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

#include <stdint.h>

#define BILLION  1000000000
#define ARM_QA7_CONTROL_REG 0x40000000
#define ARM_QA7_CONTROL_RE1 0x401BE000
#define ARM_TIMER_OFFSET 7
#define CLOCK_LSB 7
#define CLOCK_MSB 8
#define PROC_FREQ 19200000 //19.2MHz
//#define NANO_PER_CLICK 52


double getNSecPerTick();
uint64_t turnNSecToTicks(unsigned long long int nSec);

int initARMTimer();
void startARMTimer();
//void resetTestTime();
//void updateARMTimer();
//void getARMTimer(uint64_t *utime);
void setNextTime(uint64_t nTime);

int isARMTimerLessThanNext();
void waitForNext();

void printARMControl();
void printARMTime();