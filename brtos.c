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
@file   brtos.c
@author Marcelo Barros
@date   20050416

Initial stack organization for tasks:

<pre>
stack_addr 0xABCD      +------------------- +
                       | BRTOS_TaskEnd addr | -> avoid problems with tasks that return
           0xABCD - 2  +--------------------+
                       |  Status Register   | -> points to stack_addr
           0xABCD - 4  +--------------------+
                       |  Program Counter   | -> points to entry_point
           0xABCD - 6  +--------------------+
                       |                    |
                       |  Initial Context   | -> dummy values for work registers
                       |                    |
           0xABCD - 32 +--------------------+
</pre>

Register usage description:

- R0: PC
- R1: Stack Pointer
- R2: Status/CG1
- R3: CG2
- R4-R15: Working registers

If you are using msp-gcc, it is possible to optimize 
the context saving since R12, R13, R14, R15 are cloberred registers
that only need to be saved in interrupt service routines.

@todo Improvements in scheduler by adding priority capabilities

*/

#include <signal.h>
#include <io.h>
#include "brtos.h"

#define SaveContext() __asm__ __volatile__ ("push R3\n"  \
                                            "push R4\n"  \
                                            "push R5\n"  \
                                            "push R6\n"  \
                                            "push R7\n"  \
                                            "push R8\n"  \
                                            "push R9\n"  \
                                            "push R10\n" \
                                            "push R11\n" \
                                            "push R12\n" \
                                            "push R13\n" \
                                            "push R14\n" \
                                            "push R15" )

#define RestoreContext() __asm__ __volatile__ (	"pop R15\n" \
                                                "pop R14\n" \
                                                "pop R13\n" \
                                                "pop R12\n" \
                                                "pop R11\n" \
                                                "pop R10\n" \
                                                "pop R9\n"  \
                                                "pop R8\n"  \
                                                "pop R7\n"  \
                                                "pop R6\n"  \
                                                "pop R5\n"  \
                                                "pop R4\n"  \
                                                "pop R3" )

#define NUM_REGS_IN_CONTEXT   13
#define SaveStackPointer()    asm("mov.w R1,%0"  : "=m" (asBrtosTasks[ucCurrentTask].pusStackPtr))
#define RestoreStackPointer() asm("mov.w %0,R1" :: "m" (asBrtosTasks[ucCurrentTask].pusStackPtr))

#define SaveSchedStackPointer()    asm("mov.w R1,%0"  : "=m" (usSchStackPtr))
#define RestoreSchedStackPointer() asm("mov.w %0,R1" :: "m" (usSchStackPtr))

/* changing a calling function stack into a interrupt stack */
#define AdjustInterruptStack() __asm__ __volatile__ ( "decd R1\n"           \
                                                      "mov.w 2(R1),0(R1)\n" \
                                                      "mov.w R2,2(R1)")

#define ReturnFromInterrupt()  asm("reti")
#define GoToScheduler()        asm("br #BRTOS_Scheduler")

/* RTOS scheduler function is allocated at watchdog interrupt */
static interrupt (WDT_VECTOR)  BRTOS_Scheduler(void);

/** array of TCBs used to control the tasks */
static BRTOS_TCB      asBrtosTasks[BRTOS_MAX_TASKS];
/** Number of tasks */
static unsigned short usNumTasks;
/** Current priority level in execution */
static unsigned char  ucCurrentPriLevel;
/** Current task under execution */
static unsigned char  ucCurrentTask;
/** Controls if there are tasks in a specific priority level 
static unsigned char  ucNotEmptyLevel;*/
/** Scheduler stack pointer */
static unsigned short usSchStackPtr;
/** Ticks per second for RTOS. Depends on clock source and 
    interval timer configuration */
unsigned short usTicksPerSecond;

static DisableInterrupts(void)
{
	dint();
}

static EnableInterrupts(void)
{
	eint();
}

#define MSEC_TO_TICKS(x) ((usTicksPerSecond*(x))/1000)
#define TICKS_TO_MSEC(x) ((1000*(x))/usTicksPerSecond)

#define EnterCriticalSection() DisableInterrupts()
#define LeaveCriticalSection() EnableInterrupts()

/**
Generic function used to avoid a stack overflow when 
a task returns.
*/
static void BRTOS_TaskEnd(void)
{
	EnterCriticalSection();
	asBrtosTasks[ucCurrentTask].ucTaskState = BRTOS_TASK_STATE_TERMINATED;
	LeaveCriticalSection();
	while(1);
}

/**
Startup and configure the system clock. Used to define
the amount of ticks per second in RTOS. The schedule is
execute at each tick.
*/
static void BRTOS_ConfigureClock(void)
{
	/* configuring interval timer */
	WDTCTL = WDT_MDLY_0_5;
	usTicksPerSecond = 1000/0.5; /* 2k ticks per second */	
}

/**
Initialize BRTOS structures and variables.
*/
static void BRTOS_Initialize(void)
{
	int i;

	/* initialize control variables */
	ucCurrentPriLevel  = 0;
	ucCurrentTask      = 0;
	/*ucNotEmptyLevel    = 0;*/
	usNumTasks         = 0;

	/* initialize TCBs */
	for(i = 0 ; i < BRTOS_MAX_TASKS; i++)
	{
        asBrtosTasks[i].pfEntryPoint = 0;
        asBrtosTasks[i].pusStackBeg  = 0;
        asBrtosTasks[i].pusStackPtr  = 0;
        asBrtosTasks[i].usTimeSlice  = 0;
        asBrtosTasks[i].ucPriority   = 0;
        asBrtosTasks[i].ucTaskState  = BRTOS_TASK_STATE_INVALID;
        asBrtosTasks[i].usSleepTicks = 0;
        asBrtosTasks[i].usTicks      = 0;		
	}
	
    /* Configure clock: depends on external clock and clock source 
     We are assuming a 1MHz clock and the source clock as MCLK */
    BRTOS_ConfigureClock();
  
    /* Do not waste time with priorities without associated tasks 
    for(i = 0 ; i < usNumTasks ; i++)
        ucNotEmptyLevel |= asBrtosTasks[i].ucPriority;
    */
}

/**
This scheduler users a round robin schema to 
select tasks. If the time slice of current task is active, 
return the current task. Otherwise, find the next ready task
and return it. Priorities are not implemented, just the time slice feature.

@param pri seach for task in this priority level (not used yet)
@return next task to run
@todo This function could be more efficient if implemented 
      using pointers/lists instead array. Code first, optimize after.
       
*/
static int BRTOS_RoundRobin(int pri)
{
	int i, j;
	int task = BRTOS_NO_TASK_TO_RUN;

	/* starting from last task since it is the preferred task */
	/* last task must be the only task in running state ! */
	for(i = ucCurrentTask, j = 0 ; j < usNumTasks; j++, i++)
	{
		if(i >= usNumTasks) 
		  i = 0;

		/* if running, increase tick and test time slice */
		if(asBrtosTasks[i].ucTaskState == BRTOS_TASK_STATE_RUNNING)
		{
			asBrtosTasks[i].usTicks++;
			if(asBrtosTasks[i].usTicks < asBrtosTasks[i].usTimeSlice)
			{
				/* time slice not reached yet: go on */
				task = i;
				break;
			}
			else 
			{
				/* time slice reached: allow other tasks to run */
				asBrtosTasks[i].ucTaskState = BRTOS_TASK_STATE_READY;
				continue;
			}
		}
		/* task in ready state ? */
		if(asBrtosTasks[i].ucTaskState == BRTOS_TASK_STATE_READY)
		{
			asBrtosTasks[i].usTicks = 0;
			asBrtosTasks[i].ucTaskState = BRTOS_TASK_STATE_RUNNING;
			task = i;
			break;
		}
	}

	return task;
}

/**
Check timers and verify if is time to run
the task scheduler. 

The system tick should be as small and possible, to provide
a good timer resolution. Tasks, by their turn, use a bigger
periodic timer, in general a multiple of system tick.

@retval 0 timers processed, do not run the task scheduler
@retval 1 timers processed, run the task scheduler
*/
static int BRTOS_ProcessTimers(void)
{
    return 1;
}
/**
Check sleeping tasks and update their status.
*/
static int BRTOS_SleepTasks(void)
{
	int i;

	for(i = 0; i < usNumTasks; i++)
	{
		if(asBrtosTasks[i].ucTaskState == BRTOS_TASK_STATE_SLEEPING)
		{
            asBrtosTasks[i].usSleepTicks--;
            if(asBrtosTasks[i].usSleepTicks == 0)
                asBrtosTasks[i].ucTaskState = BRTOS_TASK_STATE_READY;
        }
    }
}

/**
The scheduler function.

@todo there is a problem when this function is called by
_Sleep(). We are assuming that this function is called 
only by the system tick, that it is not true in this case.
It seems that we really need a scheduler function in assembly,
with at least three different entry points:
- normal system tick processing, via interval timer
- callings coming from kernel space (no context saving)
- callings coming from user space (context saving)

\dot
digraph task_states {
node [shape=circle];
RDY -> RUN [label="next running period"]; 
RUN -> RDY [label="time slice executed"]; 
}
\enddot

*/
NAKED( BRTOS_Scheduler )
{
save_context_entry:

		SaveContext();
		SaveStackPointer();
		RestoreSchedStackPointer();
  	
dont_save_context_entry:

    if(BRTOS_ProcessTimers())
    {
      /* check sleeping tasks */
      BRTOS_SleepTasks();
		  /* Get the next task to run */
		  ucCurrentTask = BRTOS_RoundRobin(ucCurrentPriLevel);

		  if(ucCurrentTask == BRTOS_NO_TASK_TO_RUN)
		  {
			  /* nothing to do: sleep */
			  GoToLowPowerMode3();
			  goto dont_save_context_entry;
		  }

    }

    SaveSchedStackPointer();
    /* restore stack pointer */
    RestoreStackPointer();
    /* restore other registers */
    RestoreContext();	
    /* reti will pop PC and Status from stack (naked function) */
    EnableInterrupts();
    ReturnFromInterrupt();
}

/**

Initialize RTOS and user application.

*/
int main(void)
{
	/* Saving the current stack pointer. It will be used by the scheduler. */
	SaveSchedStackPointer();
  
	EnterCriticalSection();
	
	/* initialize control structures and create all user tasks */
	BRTOS_Initialize();

	/* call user initialization */
	BRTOS_Application_Initialize();
	
    /* the stack pointer should point to the first available task stack,
     and the context should be subtracted. This way, the calling of
     GoToScheduler will not fail
    */
    asBrtosTasks[ucCurrentTask].pusStackPtr += NUM_REGS_IN_CONTEXT;
    RestoreStackPointer();

    LeaveCriticalSection();
    /* go to to scheduler routine*/
    GoToScheduler();
}

/**
Create a new task.

@param entry_point Task entry point. It should be a function that follows \ref pfTaskEntry.
@param stack_addr  Start address for task stack. Remember: MSP stacks go downword and it points
                   to a valid position. 
@param time_slice Task time slice, specified in ms.
@param pri        Task priority.

@retval BRTOS_NO_ROOM_IN_TCB_ARRAY The task could not be created (no room in TCB array)
@retval BRTOS_SUCCESS The task was created sucessfully.
*/
int BRTOS_CreateTask(pfTaskEntry entry_point, unsigned short stack_addr, int time_slice, int pri)
{
    int i;
	EnterCriticalSection();
	
	if(usNumTasks >= BRTOS_MAX_TASKS)
		return BRTOS_NO_ROOM_IN_TCB_ARRAY;

	asBrtosTasks[usNumTasks].pfEntryPoint     = entry_point;
	asBrtosTasks[usNumTasks].pusStackBeg      = (unsigned short *) stack_addr;
	asBrtosTasks[usNumTasks].pusStackPtr      = (unsigned short *) stack_addr;
	asBrtosTasks[usNumTasks].usTimeSlice      = MSEC_TO_TICKS(time_slice);
	asBrtosTasks[usNumTasks].ucPriority       = pri;
	asBrtosTasks[usNumTasks].ucTaskState      = BRTOS_TASK_STATE_READY;
	asBrtosTasks[usNumTasks].usSleepTicks     = 0;
	asBrtosTasks[usNumTasks].usTicks          = 0;

	/* if task returns some day, prepare stack */
	*(asBrtosTasks[usNumTasks].pusStackPtr) = (unsigned short) BRTOS_TaskEnd;
	asBrtosTasks[usNumTasks].pusStackPtr -=1;
	
	/* status register and program counter */
	*(asBrtosTasks[usNumTasks].pusStackPtr) = (unsigned short) 0;
	asBrtosTasks[usNumTasks].pusStackPtr -=1;
	*(asBrtosTasks[usNumTasks].pusStackPtr) = (unsigned short) entry_point;
	asBrtosTasks[usNumTasks].pusStackPtr -=1;
	
    /* prepare for first RestoreContext(): dummy NUM_REGS_IN_CONTEXT regs */
    for(i = 0 ; i < NUM_REGS_IN_CONTEXT ; i++)
    {
        *(asBrtosTasks[usNumTasks].pusStackPtr) = (unsigned short) 0;
        asBrtosTasks[usNumTasks].pusStackPtr -= 1;
    }

	usNumTasks++;
	
	LeaveCriticalSection();
	
	return BRTOS_SUCCESS;
}

/**
Put the calling task to sleep (kernel level).

It is necessary to prepare the return stack and call the scheduler again.
This function cannot be called from a non task routine, otherwise it will
put the current task in execution to sleep.

Stack structure after calling:
<pre>
                    +------------------- +
                    |        ...         | -> other values in task stack
                    +--------------------+
                    |  Program Counter   | -> return address, (just after RTOS_Sleep)
  stack_pointer ->  +--------------------+
</pre>
Stack before calling scheduler should be similar to the
schedule prepared for a reti instruction:
<pre>
                    +------------------- +
                    |        ...         | -> other values in task stack
                    +--------------------+
                    |  Status Register   | -> Satus register must be saved 
                    +--------------------+
                    |  Program Counter   | -> return address (just after RTOS_Sleep)
  stack_pointer ->  +--------------------+
</pre>
*/
NAKED( _Sleep )
{
    /* reorganizing stack: PC@SP -> SR@(SP+2),PC@SP and saving context */
    AdjustInterruptStack();
    //SaveContext(); /* next operation will change the registers, so it´s 
    //                necessary to save them */
    /* setting task state */
    GoToScheduler();
}

/**
Put the calling task to sleep (user level).

@param usTime Amount of time to sleep in milliseconds
*/
void BRTOS_Sleep(unsigned short usTime)
{

    DisableInterrupts();

    /* put the task to sleep */
    asBrtosTasks[ucCurrentTask].usSleepTicks = MSEC_TO_TICKS(usTime);
    asBrtosTasks[ucCurrentTask].ucTaskState  = BRTOS_TASK_STATE_SLEEPING;

    _Sleep();
   
}

