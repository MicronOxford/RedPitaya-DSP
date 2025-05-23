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

#define TIMER_BASE_ADDRESS      0xF8F00000
#define TIMER_LOW_OFFSET        0x200
#define TIMER_HIGH_OFFSET       0x204
// #define TIMER_CONTROL_OFFSET    0x208
#define NANOSEC_PER_TICK        4

uint64_t currentTime;

int initTimer();
void updateCurrentTime();
uint64_t turnNSecToTicks(uint64_t nsec);