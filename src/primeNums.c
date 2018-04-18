/*
* Copyright (C) 2017-2018 Tiago Susano Pinto <tiagosusanopinto@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define MAXPRIME 999999
#define DONTSTOP true

bool isPrime(int n) {
    int i;
    for (i=1; i<n; ++i) {
        if (n%i==0) { return false; }
    }
    return true;
}

void printPrimesUntil (int maxPrime) {
    int k;
    for (k=2; k<=maxPrime; k++) {
        if (isPrime(k)) { printf("%u\n", k); }
    }
}

int main() {
	do {
		printPrimesUntil(MAXPRIME);
	} while (DONTSTOP);
	return 0;
}