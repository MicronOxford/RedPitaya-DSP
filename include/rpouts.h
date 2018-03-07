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

int initOuts();
void out_setpins_P(int pins);
void out_setpins_N(int pins);
void turnLEDs(int leds);
//uint32_t out_getpins();
int setPinVal(int pin, int *action, volatile uint32_t ** addr, uint32_t *val);

volatile uint32_t * getPinPDir();
volatile uint32_t * getPinNDir();