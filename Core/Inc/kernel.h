#include <stdint.h>

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV

void SVC_Handler_Main( unsigned int *svc_args );
uint32_t* stackAllocator(void);


typedef struct k_thread{
uint32_t* sp; //stack pointer
void (*thread_function)(void*); //function pointer
void *args; //Argument for thread function
uint32_t timeslice;
uint32_t runtime;
}thread;


typedef struct house{
	int windows;
	int house_number;
}house;
