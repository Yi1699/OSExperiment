#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
int main()
{
	pid_t pid, pid1;
	pid = fork();

	if(pid < 0)
	{
		fprintf(stderr,"Fork Failed");
		return 1;
	}
	else if(pid == 0)
	{
		pid1 = getpid();
		printf("child process, pid=%d\n",pid1);
		execv("./system_call",NULL);
		printf("child process, pid=%d\n",pid1);
	}
	else
	{
		pid1 = getpid();
		printf("parent process,pid=%d\n",pid1);
	}
	return 0;
}
