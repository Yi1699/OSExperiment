#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

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
		pid1 = getpid();
		printf("child:pid = %d",pid);
		printf("child:pid1 = %d",pid1);
	}
	else
	{
		wait(NULL);
		pid1 = getpid();
		printf("parent:pid = %d",pid);
		printf("parent:pid1 = %d",pid1);
	}
	return 0;
}
