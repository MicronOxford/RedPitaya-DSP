/*
* Copyright (C) 2015 Tom Parks
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
// #include <stdio.h>

typedef struct { // struct ActionLine
    uint64_t nsec; // action time, in nsec
    int pin; // pin number
    int action; // action
    uint64_t clocks; //  action time, in clock ticks
    volatile uint32_t * actAddr; // memory address relative to action
    uint32_t actVal; // value to write/check to execute the action
    // unsigned long long int executedTime; // ~time when the action was executed (CURRENTLY NOT IMPLEMENT)
} actionLine;

actionLine *actionTable;
//long int numberOfLines;

long createActionTable(char *file);