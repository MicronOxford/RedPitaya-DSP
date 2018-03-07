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

#define RP_BASE_ADDR 0x40000000

#define PIN_OFFSET   0x00
#define PINP_DIR     0x10
#define PINN_DIR     0x14
#define PINP_OUT     0x18
#define PINN_OUT     0x1C
#define LED          0x30


#define ASG_OFFSET     0x200000
#define ASG_CONFIG     0x0
#define CHA_SCALE_OFF  0x04
#define CHB_SCALE_OFF  0x24
