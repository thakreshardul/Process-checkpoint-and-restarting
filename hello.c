#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	int counter = 1;
	while(1)
	{
		printf("%d\t", counter);
		sleep(1);
		counter++;
		fflush(stdout);
	}
}