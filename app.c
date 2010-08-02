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
@file   app.c
@author Marcelo Barros
@date   20050416

*/

#include <io.h>
#include "brtos.h"

/* at least 32 bytes for this implementation */
#define TASK_STACK_SIZE 40

/* 
  Using unsigned short to force the alignment (it is necessary
  to check if this information is true for MSP430)
*/
unsigned short usStack_a [TASK_STACK_SIZE/2];
unsigned short usStack_b [TASK_STACK_SIZE/2];

void task_a(unsigned long ulArgc)
{
	int i = 0;
    while(1)
    {
        while(i < 50)
        {
            i = i + 1;
        }
        BRTOS_Sleep(5);
    }
}

void task_b(unsigned long ulArgc)
{
	int i = 0;
    while(1)
    {
        while(i < 100)
        {
            i = i + 1;
        }
        BRTOS_Sleep(10);
    }
}


/*
Schedule implemented:
- 10 ms running task a
-  5 ms waiting
- 20 ms running task b
- 10 ms waiting
*/
void BRTOS_Application_Initialize(void)
{
	BRTOS_CreateTask(task_a,(unsigned short)&usStack_a [TASK_STACK_SIZE/2-1],10,BRTOS_TASK_PRIORITY_1);
	BRTOS_CreateTask(task_b,(unsigned short)&usStack_b [TASK_STACK_SIZE/2-1],20,BRTOS_TASK_PRIORITY_1);
}
