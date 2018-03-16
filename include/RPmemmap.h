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


// This values are based on http://rpdocs.readthedocs.io/en/latest/developerGuide/software/fpga.html
#define RP_BASE_ADDR 0x40000000

// GPIOs & LEDs
#define PIN_OFFSET   0x00
/* Expansion connector direction P/N    (1-out 0-in)
[31:8]  Reserved                        (Read Only)
[7:0]   Direction for P/N lines         (R/W)*/
#define PINP_DIR    0x10
#define PINN_DIR    0x14
/* Expansion connector output P/N
[31:8]  Reserved                        (Read Only)
[7:0]   P/N pins output                 (R/W) */
#define PINP_OUT    0x18
#define PINN_OUT    0x1C
/* Expansion connector input P/N
[31:8]  Reserved                        (Read Only)
[7:0]   P/N pins input                  (Read Only)*/
#define PINP_IN     0x20
#define PINN_IN     0x24
/*LED Control
[31:8]  Reserved                        (Read Only)
[7:0]   LEDs 7-0                        (R/W)*/
#define LED         0x30

// Analogue outputs
// ASG - Arbitrary Signal Generator
#define ASG_OFFSET     0x200000
/* Configuration ([31:16] Channel B; [15:0] Channel A)
CHANNEL B
[31:25] Reserved                        (Read Only)
[24]    external gated repetitions      (R/W)
[23]    set output to 0                 (R/W)
[22]    State Machine reset             (R/W)
[21]    Reserved                        (R/W)
[20]    SM wrap pointer                 (R/W)
        (if disabled starts at address 0)
[19:16] Trigger selector                (R/W)
        (don’t change when SM is active)
            1-trig immediately
            2-external trigger positive edge
            3-external trigger negative edge
CHANNEL A
[15:9]  Reserved                        (Read Only)
[8]     external gated repetitions      (R/W)
[7]     set output to 0                 (R/W)
[6]     State Machine reset             (R/W)
[5]     Reserved                        (R/W)
[4]     SM wrap pointer                 (R/W)
        (if disabled starts at address 0)
[3:0]   Trigger selector                (R/W)
        (don’t change when SM is active)
            1-trig immediately
            2-external trigger positive edge
            3-external trigger negative edge*/
#define ASG_CONFIG     0x0
/*Channel A amplitude scale and offset
    output = (data*scale)/0x2000 + offset       
[31:30] Reserved                    (Read Only)
[29:16] Amplitude offset            (R/W)
[15:14] Reserved                    (Read Only)
[13:0]  Amplitude scale (Unsigned)  (R/W)
            0x2000 == multiply by 1
*/
#define CHA_SCALE_OFF  0x04
/*Channel B amplitude scale and offset
    output = (data*scale)/0x2000 + offset       
[31:30] Reserved                    (Read Only)
[29:16] Amplitude offset            (R/W)
[15:14] Reserved                    (Read Only)
[13:0]  Amplitude scale (Unsigned)  (R/W)
            0x2000 == multiply by 1
*/
#define CHB_SCALE_OFF  0x24
