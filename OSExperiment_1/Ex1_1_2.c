#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int value = 0;

int main()
{
	pid_t pid, pid1;
	pid = fork();

	if(pid < 0)
	{
		fprintf(stderr, "Fork Failed");
		return 1;
	}
	else if(pid == 0)
	{
        	value += 2;
		pid1 = getpid();
		printf("child:value = %d value's add=%d\n",value,&value);
	}
	else
	{
        	value -= 2;
		pid1 = getpid();
		printf("parent:value = %d value's add=%d\n",value,&value);
	}
	value++;
	printf("before return value=%d,value's add=%d\n",value,&value);
	return 0;
}
