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


// The following values are based on
// http://rpdocs.readthedocs.io/en/latest/developerGuide/software/fpga.html
#define RP_BASE_ADDR    0x40000000

// GPIOs & LEDs
#define PIN_OFFSET      0x00
/* Expansion connector direction P/N    (1-out 0-in)
[31:8]  Reserved                        (Read Only)
[7:0]   Direction for P/N lines         (R/W)*/
#define DIRECT_PINP     0x10
#define DIRECT_PINN     0x14
/* Expansion connector output P/N
[31:8]  Reserved                        (Read Only)
[7:0]   P/N pins output                 (R/W)*/
#define OUT_PINP        0x18
#define OUT_PINN        0x1C
/* Expansion connector input P/N
[31:8]  Reserved                        (Read Only)
[7:0]   P/N pins input                  (Read Only)*/
#define IN_PINP         0x20
#define IN_PINN         0x24
/*LED Control
[31:8]  Reserved                        (Read Only)
[7:0]   LEDs 7-0                        (R/W)*/
#define LED             0x30

// Analogue outputs (OUT 1/2 = Channel A/B)
// ASG - Arbitrary Signal Generator
#define ASG_OFFSET      0x200000
/* Configuration ([31:16] ChannelB; [15:0] ChannelA)
CHANNEL B
[31:25] Reserved                        (Read Only)
[24]    External gated repetitions      (R/W)
[23]    Set output to 0                 (R/W)
[22]    SM (State Machine) reset        (R/W)
[21]    Reserved                        (R/W)
[20]    SM (State Machine) wrap pointer (R/W)
        (if disabled starts at address 0)
[19:16] Trigger selector                (R/W)
        (don’t change when SM is active)
            1-trig immediately
            2-external trigger positive edge
            3-external trigger negative edge
CHANNEL A
[15:9]  Reserved                        (Read Only)
[8]     External gated repetitions      (R/W)
[7]     Set output to 0                 (R/W)
[6]     SM (State Machine) reset        (R/W)
[5]     Reserved                        (R/W)
[4]     SM (State Machine) wrap pointer (R/W)
        (if disabled starts at address 0)
[3:0]   Trigger selector                (R/W)
        (don’t change when SM is active)
            1-trig immediately
            2-external trigger positive edge
            3-external trigger negative edge*/
#define ASG_CONFIG      0x0
#define ASG_CONFIG_NOTRESERVED 0x01DF01DF
#define ASG_CONFIG_SMRESET_CHA 22
#define ASG_CONFIG_SMRESET_CHB 6
/*Channel A/B amplitude scale and offset
    output = (data*scale)/0x2000 + offset       
[31:30] Reserved                        (Read Only)
[29:16] Amplitude offset                (R/W)
[15:14] Reserved                        (Read Only)
[13:0]  Amplitude scale (Unsigned)      (R/W)
            0x2000 == multiply by 1*/
#define SCALEOFF_CHA    0x04
#define SCALEOFF_CHB    0x24
#define SCALEOFF_NOTRESERVED 0x3FFF3FFF
#define SCALEOFF_OFFSET 16
#define SCALEOFF_SCALE  0
/*Channel A/B counter wrap
[31:30] Reserved                        (Read Only)
[29:0]  Where counter wraps around      (R/W)
        Depends on State Machine wrap setting:
            0 - counter goes to offset value
            1 - new value is get by wrap
                (16 bits for decimals)*/
#define COUNTWRAP_CHA   0x08
#define COUNTWRAP_CHB   0x28
/*Channel A/B start offset        
[31:30] Reserved                        (Read Only)
[29:0]  Counter start offset            (R/W)
        Start offset when trigger arrives
            (16 bits for decimals)*/
#define STARTOFF_CHA    0x0C
#define STARTOFF_CHB    0x2C
/*Channel A/B counter step        
[31:30] Reserved                        (Read Only)
[29:0]  Counter step                    (R/W)
            (16 bits for decimals)*/
#define COUNTERSTEP_CHA 0x10
#define COUNTERSTEP_CHB 0x30
/*Channel A/B buffer current read pointer         
[31:16] Reserved                        (Read Only)
[15:2]  Read pointer                    (R/W)
[1:0]   Reserved                        (Read Only)*/
#define READPOINT_CHA   0x14
#define READPOINT_CHB   0x34
/*Channel A/B number of read cycles in one burst          
[31:16] Reserved                        (Read Only)
[15:0]  Num of repeats of table readout (R/W) 
            (0 = infinite)*/
#define READCYCLE_CHA   0x18
#define READCYCLE_CHB   0x38
/*Channel A/B number of burst repetitions         
[31:16] Reserved                        (Read Only)
[15:0]  Number of repetitions           (R/W)
            (0 = disabled)*/
#define NUMBURST_CHA    0x1C
#define NUMBURST_CHB    0x3C
/*Channel A/B delay between burst repetitions         
[31:0]  Delay between repetitions       (R/W)
            (Granularity = 1us)*/
#define DELAYBURST_CHA  0x20
#define DELAYBURST_CHB  0x40

/*Channel A/B memory data (16k samples)
[31:14] Reserved                        (Read Only)
[13:0]  Channel A/B data                (R/W)*/
#define OUTPUT_CHA      0x10000 //to 0x1FFFC
#define OUTPUT_CHB      0x20000 //to 0x2FFFC