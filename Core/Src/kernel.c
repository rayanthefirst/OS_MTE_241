#include "kernel.h"
#include <stdio.h>
#include <stdint.h>
#include "main.h"

unsigned int svc_number;
#define stackPool 0x4000
#define stackSize 0x400
#define RUN_FIRST_THREAD 0x3
extern void runFirstThread(void);
#define YIELD 4

int interesting_value = 0xBA5EBA11;
uint32_t* MSP_INIT_VAL;
uint32_t* lastAllocatedStack;
uint32_t usedStackPool;
thread stackThreads[stackPool / stackSize];
int currentThread;
int threadCount;
//int kernelStarted = 0;


void osYield(void){
	__asm("SVC #4");
}
void osSched(){
	stackThreads[currentThread].sp = (uint32_t*)(__get_PSP() - 8*4);
	currentThread = (currentThread+1)%threadCount;
	__set_PSP(stackThreads[currentThread].sp);
	return;}

void osKernelInitialize(void){
	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV
	MSP_INIT_VAL = *(uint32_t**)0x0;
	lastAllocatedStack = MSP_INIT_VAL;
	usedStackPool = 0;
	currentThread = 0;
	threadCount = 0;
//	kernelStarted = 1;
	return;
}

void osKernelStart(void) {
	__asm("SVC #3");
}
int osCreateThread (void (*func)(void), void *args){
	uint32_t* stackptr = stackAllocator();
	if (stackptr == NULL){return 0;}
	*(--stackptr) = 1<<24; //A magic number, this is xPSR
	*(--stackptr) = (uint32_t)func; //the function name
	for (int i = 0; i < 6; i++) {
		--stackptr;
	}
	*(stackptr) = args;
	for (int i =0; i<8; i++) {
		*(--stackptr) = 0xA; //An arbitrary number
	  }

	stackThreads[threadCount].timeslice = 200;
	stackThreads[threadCount].runtime = 200;
	stackThreads[threadCount].sp = stackptr;
	stackThreads[threadCount].thread_function = func;
	threadCount++;

	return 1;
}

int osCreateThreadWithDeadline (void (*func)(void), void *args, uint32_t timeslice, uint32_t runtime){
	uint32_t* stackptr = stackAllocator();
	if (stackptr == NULL){return 0;}
	*(--stackptr) = 1<<24; //A magic number, this is xPSR
	*(--stackptr) = (uint32_t)func; //the function name
	for (int i = 0; i < 6; i++) {
		--stackptr;
	}
	*(stackptr) = args;
	for (int i =0; i<8; i++) {
		*(--stackptr) = 0xA; //An arbitrary number
	  }

	stackThreads[threadCount].timeslice = timeslice;
	stackThreads[threadCount].runtime = runtime;
	stackThreads[threadCount].sp = stackptr;
	stackThreads[threadCount].thread_function = func;
	threadCount++;

	return 1;
}

uint32_t* stackAllocator(void) {
	if (usedStackPool >= 0x4000) {
		return NULL;
	}

	lastAllocatedStack -= stackSize;
	usedStackPool += stackSize;

	return lastAllocatedStack;
}


void SVC_Handler_Main( unsigned int *svc_args )
{
/*
* Stack contains:
* r0, r1, r2, r3, r12, r14, the return address and xPSR
* First argument (r0) is svc_args[0]
*/
svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
switch( svc_number )
{
case 17: //17 is sort of arbitrarily chosen
	printf("Success!\r\n");
	break;
case 18:
	printf("18 - system call\r\n");
	break;
case 19:
	printf("19 - system call");
	break;
case 3:
	__set_PSP(stackThreads[currentThread].sp);
	runFirstThread();
	break;

case 4:
	//Pend an interrupt to do the context switch
	_ICSR |= 1<<28;
	__asm("isb");
	break;

default: /* unknown SVC */
	printf("fail");
	break;
}
}
