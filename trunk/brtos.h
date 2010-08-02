/*
Copyright 2005- Marcelo Barros de Almeida. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, 
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice, 
      this list of conditions and the following disclaimer in the documentation 
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE BASIC RTOS PROJECT ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE BASIC RTOS PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the authors and should not be interpreted as representing official policies, either expressed or implied, of the Basic RTOS Project.

*/

/**
@file   brtos.h
@author Marcelo Barros
@date   20050416

*/
#ifndef __BRTOS_H__
#define __BRTOS_H__


#define BRTOS_MAX_PRIORITY_LEVELS  8
#define BRTOS_MAX_TASKS            5
#define BRTOS_NO_TASK_TO_RUN      (BRTOS_MAX_TASKS+1)

/* eight priority levels */
#define BRTOS_TASK_PRIORITY_1 0x01
#define BRTOS_TASK_PRIORITY_2 0x02
#define BRTOS_TASK_PRIORITY_3 0x04
#define BRTOS_TASK_PRIORITY_4 0x08
#define BRTOS_TASK_PRIORITY_5 0x10
#define BRTOS_TASK_PRIORITY_6 0x20
#define BRTOS_TASK_PRIORITY_7 0x40
#define BRTOS_TASK_PRIORITY_8 0x80

/* task states */
#define BRTOS_TASK_STATE_INVALID    0x00
#define BRTOS_TASK_STATE_RUNNING    0x01
#define BRTOS_TASK_STATE_READY      0x02
#define BRTOS_TASK_STATE_SLEEPING   0x04
#define BRTOS_TASK_STATE_TERMINATED 0x08
#define BRTOS_TASK_STATE_WAITING    0x10

#define BRTOS_TASK_NULL              0x00

/* return codes */
#define BRTOS_SUCCESS              0x00
#define BRTOS_FAILURE              0x01
#define BRTOS_NO_ROOM_IN_TCB_ARRAY 0x02

/* go to saving energy mode 3 */
#define GoToLowPowerMode3()  LPM3

typedef void (*pfTaskEntry)(unsigned long); /* task entry: void task(unsigned long) */

typedef struct {
	pfTaskEntry    pfEntryPoint;      /* task entry point    */
	unsigned char  ucPriority;        /* task priority       */
	unsigned char  ucTaskState;       /* current task state  */
	unsigned short usTimeSlice;       /* desired time slice  */
	unsigned short *pusStackBeg;      /* stack beginning     */
	unsigned short *pusStackPtr;      /* stack pointer       */
	unsigned short usSleepTicks;      /* count sleep ticks   */
	unsigned short usTicks;           /* count slice ticks   */
} BRTOS_TCB;

typedef struct {
	pfTaskEntry    pfEntryPoint;
	unsigned short usCounter;
	unsigned char  ucPriority;
	unsigned char  ucStatus;
} BRTOS_TIMER;

/* prototypes */
int BRTOS_CreateTask(pfTaskEntry entry_point, unsigned short stack_addr, int time_slice, int pri);
void BRTOS_Sleep(unsigned short usTime);
extern void BRTOS_Application_Initialize(void);

#endif /* __BRTOS_H__ */
