#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define MAXPRIME 999999

bool isPrime(int n) {
	int i;
	for (i=n-1; i>1; --i){
  		if (n%i==0) { return false; }
  	}
  	return true;
}

void printPrimesUntil (int maxPrime) {
  int k;

  for(k=0; k<=maxPrime; k++){
  	if(isPrime(k)){
  		printf("%u\n", k);
  	}
  }
}

int main() {
	while(true) {
		printPrimesUntil(MAXPRIME);
	}
	return 0;
}