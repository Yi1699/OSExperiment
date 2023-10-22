#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
void* thread_1();
void* thread_2();
int main()
{
	pthread_t pid1, pid2;
	int res1, res2;
	res1 = pthread_create(&pid1, NULL, thread_1, NULL);
	if(res1){
		printf("create failed!\n");
		return 0;
	}else printf("thread1 create success!\n");
	res2 = pthread_create(&pid2, NULL, thread_2, NULL);
	if(res2){	
		printf("create failed!\n");
		return 0;
	}else printf("thread2 create success!\n");
	pthread_join(pid1, NULL);
	pthread_join(pid2, NULL);
	return 0;
}
void* thread_1()
{
	int pid = getpid();
	int tid = syscall(SYS_gettid);
	printf("thread1 tid = %d, pid = %d\n", tid, pid);
	system("./system_call");
	printf("thread1 system_call return!\n");
	return 0;
}
void* thread_2()
{
	int pid = getpid();
	int tid = syscall(SYS_gettid);
	printf("thread2 tid = %d, pid = %d\n", tid, pid);
	system("./system_call");
	printf("thread2 system_call return!\n");
	return 0;
}
