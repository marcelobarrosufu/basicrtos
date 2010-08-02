/**
\mainpage Basic RTOS

<center>
<h2>Marcelo Barros de Almeida</h2>
<h3>marcelobarrosalmeida@gmail.com</h3>
<h3>http://code.google.com/p/basicrtos/</h3>
</center>

\section SECDESC Description

Basic RTOS (BRTOS) is a non-preemptive real time operating system for MSP430 microcontrollers.

Main features:

- Non-preemptive
- Round robin as task scheduling 
- Tasks with time slice support
- Interrupts are not handled by BRTOS at this moment (user must disable interrupts when running an interrupt routine)

Current limitations:

- Only support MSP430
- Task priorities are not implemented yet
- Timers and semaphores not implemented yet

\section SECDCRED Credits

Original author:
Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>

\section SECLIC License

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

