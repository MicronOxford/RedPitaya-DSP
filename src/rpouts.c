/*
* Copyright (C) 2015 Tom Parks
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <byteswap.h>
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

int OUTS_FD;
volatile uint8_t *OUTS_MMAP;

int initOuts(){
  OUTS_FD = open("/dev/mem", O_RDWR|O_SYNC);
  if (OUTS_FD < 0) {
      fprintf(stderr, "open(/dev/mem) failed (%d)\n", errno);
      return 1;
  }

  // map the whole of the RP FPGA space
  OUTS_MMAP = mmap(NULL, PAGE_SIZE*2048, PROT_READ | PROT_WRITE, MAP_SHARED, OUTS_FD, RP_BASE_ADDR);
  if (OUTS_MMAP == MAP_FAILED) {
      fprintf(stderr, "mmap64(0x%x@0x%x) failed (%d)\n",
              PAGE_SIZE, (uint32_t)(RP_BASE_ADDR), errno);
      return 1;
  }

  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_DIR) = 0xFFFFFFFF;
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_DIR) = 0xFFFFFFFF;
  out_setpins_P(0);
  out_setpins_N(0);
  return 0;
}

inline void out_setpins_P(int pins){
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT) = pins;
}

inline void out_setpins_N(int pins){
  *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINN_OUT) = pins;
}

uint32_t out_getpins(){
  return *( volatile uint32_t *)(OUTS_MMAP+PIN_OFFSET+PINP_OUT);
}
