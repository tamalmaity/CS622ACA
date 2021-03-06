   /*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2018 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include <stdio.h>
#include <inttypes.h>
#include<string.h>
#include "pin.H"


FILE * trace;
PIN_LOCK pinLock;

static UINT64 mem_acc = 0;
unsigned long long count = 0;


// This routine is executed every time a thread is created.
VOID ThreadStart(THREADID tid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    PIN_GetLock(&pinLock, tid+1);
    fprintf(stdout, "thread begin %d\n",tid);
    fflush(stdout);
    PIN_ReleaseLock(&pinLock);
}

// This routine is executed every time a thread is destroyed.
VOID ThreadFini(THREADID tid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    PIN_GetLock(&pinLock, tid+1);
    fprintf(stdout, "thread end %d code %d\n",tid, code);
    fflush(stdout);
    PIN_ReleaseLock(&pinLock);
}


// Print a memory read record
VOID RecordMemRead(VOID * addr, THREADID tid, UINT32 size, INT32 mode)
{   
	PIN_GetLock(&pinLock, tid+1);

    long long unsigned address = (long long unsigned) addr;

    int arr[] = {0,0,0,0}; //containing number of 1's, 2's, 4's and 8's

    if (address%64!=0)
    {
        if(((address/64)+1)*64 < address + size)
        {
            int to_fit = ((address/64)+1)*64 - address;

            int eight = to_fit/8;
            arr[3]+= eight;
            size-= (eight*8);
            to_fit = to_fit%8;

            int four = to_fit/4;
            arr[2]+= four;
            size-= (four*4);
            to_fit = to_fit%4;

            int two = to_fit/2;
            arr[1]+= two;
            size-= (two*2);
            to_fit = to_fit%2;

            if(to_fit%2!=0)
            {
                arr[0]+= 1;
                size-= 1;
            }


        }   
    }

    int eight = size/8;
    arr[3]+= eight;
    size-= (eight*8);

    int four = size/4;
    arr[2]+= four;
    size-= (four*4);

    int two = size/2;
    arr[1]+= two;
    size-= (two*2);

    if (size!=0)
    {
        arr[0]+= 1;
        size-= 1;
    }

    for (int i=0;i<arr[3];i++)
    {
        fprintf(trace,"%llu %d %llu 8 %d\n", count, tid,  address, mode);
        address+= 8;
        count++;
    }

    for (int i=0;i<arr[2];i++)
    {
        fprintf(trace,"%llu %d %llu 4 %d\n", count, tid,  address, mode);
        address+= 4;
        count++;
    }
    for (int i=0;i<arr[1];i++)
    {
        fprintf(trace,"%llu %d %llu 2 %d\n",count, tid,  address, mode);
        address+= 2;
        count++;
    }
    for (int i=0;i<arr[0];i++)
    {
        fprintf(trace,"%llu %d %llu 1 %d\n",count, tid,  address, mode);
        address+= 1;
        count++;
    }

    fflush(trace);
    PIN_ReleaseLock(&pinLock);
}
// Print a memory write record
VOID RecordMemWrite(VOID * addr, THREADID tid, UINT32 size, INT32 mode)
{
	PIN_GetLock(&pinLock, tid+1);
    long long unsigned address = (long long unsigned) addr;

    int arr[] = {0,0,0,0}; //containing number of 1's, 2's, 4's and 8's

    if (address%64!=0)
    {
        if(((address/64)+1)*64< address + size)
        {
            int to_fit = ((address/64)+1)*64 - address;

            int eight = to_fit/8;
            arr[3]+= eight;
            size-= (eight*8);
            to_fit = to_fit%8;

            int four = to_fit/4;
            arr[2]+= four;
            size-= (four*4);
            to_fit = to_fit%4;

            int two = to_fit/2;
            arr[1]+= two;
            size-= (two*2);
            to_fit = to_fit%2;

            if(to_fit%2!=0)
            {
                arr[0]+= 1;
                size-= 1;
            }


        }   
    }

    int eight = size/8;
    arr[3]+= eight;
    size-= (eight*8);

    int four = size/4;
    arr[2]+= four;
    size-= (four*4);

    int two = size/2;
    arr[1]+= two;
    size-= (two*2);

    if (size!=0)
    {
        arr[0]+= 1;
        size-= 1;
    }

    for (int i=0;i<arr[3];i++)
    {
        fprintf(trace,"%llu %d %llu 8 %d\n", count, tid,  address, mode);
        address+= 8;
        count++;
    }

    for (int i=0;i<arr[2];i++)
    {
        fprintf(trace,"%llu %d %llu 4 %d\n", count, tid,  address, mode);
        address+= 4;
        count++;
    }
    for (int i=0;i<arr[1];i++)
    {
        fprintf(trace,"%llu %d %llu 2 %d\n",count, tid,  address, mode);
        address+= 2;
        count++;
    }
    for (int i=0;i<arr[0];i++)
    {
        fprintf(trace,"%llu %d %llu 1 %d\n",count, tid,  address, mode);
        address+= 1;
        count++;
    }
    fflush(trace);
    PIN_ReleaseLock(&pinLock);
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    mem_acc+= memOperands;

    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {   
    	UINT32 refSize = INS_MemoryOperandSize(ins, memOp);

        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_MEMORYOP_EA, memOp,
                IARG_THREAD_ID,
                IARG_UINT32,refSize,
                IARG_UINT32, 0,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_MEMORYOP_EA, memOp,
                IARG_THREAD_ID,
                IARG_UINT32,refSize,
                IARG_UINT32, 1,
                IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID *v)
{
    //fprintf(trace, "#eof\n");
    fclose(trace);
    //printf("%llu\n",count);
   // printf("Number of memory accesses = %lu\n", mem_acc);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{	char file_prefix[100];
	int number;
//char c;
 	printf("Enter number of prog file on which you are running the program: ");
	scanf("%d",&number);

 	snprintf(file_prefix, 30, "mach_acc_%d.txt", number); 
    trace = fopen(file_prefix, "w");
    PIN_InitLock(&pinLock);
    if (PIN_Init(argc, argv)) return Usage();

    INS_AddInstrumentFunction(Instruction, 0);
    
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
