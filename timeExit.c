#include "stdio.h"
#include "unistd.h"
#include "assert.h"
#include "stdlib.h"
#include <sys/syscall.h>
#define __USE_GNU
#include "sched.h"
#include <inttypes.h>
void usage(){
	printf("use -v to specify number of vm exits to cause\n");
}
__inline__ uint64_t
get_tsc(void) {
	uint32_t lo, hi;
	//if rdtscp is not available on your system, change to rdtsc
	//and add a cpuid instruction before (for serialization)
	//make sure its __volatile__ flagged or compiler will rudely
	//move it around
	__asm__ __volatile__ ("rdtscp" : "=a" (lo), "=d" (hi));
	return (uint64_t)hi << 32 | lo;
}

//Code is written for readability..
int main(int argc, char ** argv){

	if(argc < 2){
		printf("pass in the number of trials\n");
		return 0;
	}
	uint64_t ntimes_per_measurement, totalCycles, time_per_exit, numVMExits, exit_begin, exit_end, overhead_end, overhead_begin;
	int numTrials = atoi(argv[1]);
	ntimes_per_measurement = 10;
	uint64_t results[numTrials]; 
	int i = 0;

	while(numTrials--){
		numVMExits = ntimes_per_measurement;
		exit_begin = get_tsc();
		
		//time 1mil hypercalls to get vmexit cost
		while(numVMExits--){
			syscall(314,0);
		}
		exit_end = get_tsc();
		numVMExits = ntimes_per_measurement;

		//Time the overhead of making a syscall
		//syscall 315 is basically a noop syscall
		overhead_begin = get_tsc();
		while(numVMExits--){
			syscall(315,0);
		}
		overhead_end = get_tsc();

		//total cycles is number of cycles for vmexits - number for noop syscall
		totalCycles = (exit_end - exit_begin) - (overhead_end - overhead_begin);
		time_per_exit = totalCycles / ntimes_per_measurement;
		results[i++] = time_per_exit;
	}

	//dump output in csv format
	while(i--){
		printf("%" PRIu64 "",results[i]);
		if(i) printf(",");
	}
	return 0;
}
