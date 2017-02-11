#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ucontext.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <dlfcn.h>

#include "parser.h"


void signal_handler (int signal)
{
	if (signal == SIGUSR2)
	{
		int process, checkpoint;
		MemoryRegion *proc = (MemoryRegion *)malloc(sizeof(MemoryRegion));
		ucontext_t context, *cp=&context;
		process = open("/proc/self/maps", O_RDONLY);
		if (process == -1)
			fprintf(stderr, "Error openeing /proc/self/maps:%s\n", strerror(errno));
		checkpoint = open("myckpt", O_APPEND | O_WRONLY | O_CREAT, S_IRWXU);
		
		if (checkpoint == -1)
			fprintf(stderr,"Error opening checkpoint%s\n", strerror(errno));
		while(1)
		{
			if (readline(process, proc) == 0)
				break;
				
			if ((proc->isReadable == 1) && (!(strstr(proc->name, "vsys"))))
			{	
				if (write(checkpoint, proc, sizeof(MemoryRegion)) == -1) 
				{
					fprintf(stderr, "Unable to write struct%s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}
				ssize_t wrote = write(checkpoint, proc->startAddr, proc->size * sizeof (char));
				if (wrote == -1) 
				{
					fprintf(stderr, "Unable to write data%s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}
			
				
			}
  		}
  		free(proc);	
  		int pid = getpid();
  		if(getcontext(cp) == -1)
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if (getpid() != pid)
		{
			return;
		}
		
		ssize_t bytes = write(checkpoint, &context, sizeof(ucontext_t));
		if (!bytes)
		{
			fprintf(stderr, "Context not written%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		close(checkpoint);
	}
	printf("\n");
	exit(0);
	
}

__attribute__ ((constructor))
void checkpoint_constructor(){
	signal(SIGUSR2, signal_handler);
}
