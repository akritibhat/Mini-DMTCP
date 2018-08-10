# Mini-DMTCP

This is a mini-DMTCP. DMTCP is a widely used software package for checkpoint-restart: http://dmtcp.sourceforge.net. 
Given an arbitrary application in C or C++, you can run shared library, libckpt.so by:
LD_PRELOAD=/path/to/libckpt.so ./myprog
where /path/to/libckpt.so must be an absolute pathname for the libckpt.so.


The program reads /proc/self/maps (which is the memory layout of the current process) and save that information in a checkpoint image.

The image has information about the entire process. This includes saving the registers.
For each memory section, we save information that we need to later restore the memory:
a section header (including address range of that memory, and r/w/x permissions)
the data in that address range.

We trigger the checkpoint by writing a signal handler for SIGUSR2

To trigger a checkpoint from a command line:
 kill -12 PID 
 
where PID is the process id of the target process that we wish to checkpoint. (Signal 12 is SIGUSR2, reserved for end users.

The signal handler helps when we call signal() to register the signal handler.
The call to signal() is inside libckpt.so, hence, it is called before the main() routine of the end user. 
The solution is to define a constructor function in C inside libckpt.so. 

The name of checkpoint image file is myckpt.


Restoring from a checkpoint image file :
Using program myrestart.c, and compiling it statically at an address that the target process is not likely to use.
For example:
 gcc -static \
        -Wl,-Ttext-segment=5000000 -Wl,-Tdata=5100000 -Wl,-Tbss=5200000 \
        -o myrestart myrestart.c 
		
The gcc syntax -Wl,Ttext=... means pass on the parameter -Ttext=... to the linker that will be called by gcc. 

When myrestart begins executing, it will moves its stack to some infrequently used address, like 0x5300000. 

The myrestart.c program should takes one argument, the name of the checkpoint image file from which to restore the process. 

Now copy that filename from the stack (probably from 'argv[1]') into the data segment (probably into a global variable). 
(We will soon be unmapping the initial stack. So, we better first save any important data.) 
When we copy the filename into allocated storage in your data segment, an easy way is to declare a global variable and then copy to the global variable.

Using the inline assembly syntax of gcc to include code we will switch the stack pointer (syntax: $sp or %sp) from the initial stack to the new stack,
then immediately make a function call to a new function. The new function will then use a call frame in the new stack. 

Unmap the stack used by the myrestart program. The reason is that the stack might be located at an address range 
that conflicts with what the target program was using prior to checkpointing. Since the goal is to restore the memory 
as it existed at checkpoint time, the conflicting addresses can lead to hard to debug problems.

To unmap the stack:
read the /proc/self/maps of the myrestart process;
search for the region with the name [stack] denoting the current stack in use; and
call munmap on the stack region.

Copy the registers from the file header of checkpoint image file into some pre-allocated memory in data segment.

The memory of myrestart process should no longer conflict with the memory to be restored from the checkpoint image. 
Now read the addresses of each memory segment from checkpoint image file. 
It's in  section header. 
Call mmap to map into memory a corresponding memory section for new process. 
Copy the data from the memory sections of  checkpoint image into the corresponding memory sections.

Now,  need to jump into the old program and restore the old registers. 
In a previous step, we had copied the old registers into data segment.
Now use setcontext() to restore those as the working registers. 
This will restore  program counter (pc) and stack pointer (sp) and automatically start using the text segment and 
stack segment from your previously checkpointed process.
