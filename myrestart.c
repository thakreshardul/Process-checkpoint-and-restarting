#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ucontext.h>

#include "parser.h"

char cktp_image[1024] = "\0";

void restore_memory()
{
	MemoryRegion cktp_proc, old_proc;
	ucontext_t ucp, *cp = &ucp;
	int checkpoint, counter=0, flags = MAP_ANONYMOUS, prot;
	size_t nbytes = sizeof(MemoryRegion);
	VA memory, test;
	checkpoint = open(cktp_image, O_RDONLY);
	int this_process = open("/proc/self/maps", O_RDONLY);
	while(1)
	{
		if (readline(this_process, &old_proc) == 0)
		{
			//perror("error reading /proc/self/maps");
			break;
		}
		if (strstr(old_proc.name, "[stack]"))
		{
			printf("\n");
			int q = munmap(old_proc.startAddr, old_proc.size);	
		}
	}
	close(this_process);
	while(counter < 20)
	{
		prot = PROT_WRITE;
		int ret_val = read(checkpoint, &cktp_proc, nbytes);
   		if (ret_val == -1) 
   		{
    		fprintf(stderr, "Unable to read file: %s\n",strerror(errno));
      		exit(EXIT_FAILURE);
		}
		if (ret_val == 0)
			break;

		if (cktp_proc.isReadable)
			prot |= PROT_READ;
		if (cktp_proc.isWritable)
			prot |= PROT_WRITE;
		if (cktp_proc.isExecutable)
			prot |= PROT_EXEC;
		if (cktp_proc.isPrivate)
			flags |= MAP_PRIVATE;
		else	 
			flags |= MAP_SHARED;
		if (strstr(cktp_proc.name,"[stack]"))
		{
			flags |= MAP_STACK;
			test = cktp_proc.startAddr;
		}

		memory = mmap(cktp_proc.startAddr, cktp_proc.size, 
    		prot, flags, -1, cktp_proc.offset);
		
		if (memory == MAP_FAILED)
		{
			fprintf(stderr, "Cannot map %s:%s\n", cktp_proc.name, strerror(errno));
			exit(EXIT_FAILURE);
		}

		char *proc_data = (char *)malloc(cktp_proc.size);
		ssize_t a = read(checkpoint, proc_data, cktp_proc.size);
		if (!a)
		{
			fprintf(stderr, "Could not read data%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	
		memcpy(memory, proc_data, cktp_proc.size);

	  	free(proc_data); 
	  	counter++;
	}

	size_t z =read(checkpoint, &ucp, sizeof (ucontext_t));
	if (z == -1)
	{
		fprintf(stderr, "%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	setcontext(cp);
}

void main(int argc, char const *argv[])
{
	MemoryRegion proc;
	VA memory, new_stackAddr = 0x5300000;
	int process = open("/proc/self/maps", O_RDONLY);
	if (!argv[1])
	{
		perror("No checkpoint file specified");
		exit(1);
	}
	strcpy(cktp_image,argv[1]);
	while(1)
	{

		if (readline(process, &proc) == 0)
		{
			perror("error reading /proc/self/maps");
		}
		if (strstr(proc.name, "[stack]"))
		{
			close(process);
			memory = mmap(new_stackAddr, proc.size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_GROWSDOWN | MAP_ANONYMOUS, -1, 0);
			if (memory == MAP_FAILED)
			{
				perror("Can't map memory");
				printf("%s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}

			asm ("mov %[sp], %%rsp;"
				: 
				: [sp] "r" (memory)
				: "memory");
			
			restore_memory();
			
		}		
	}
}