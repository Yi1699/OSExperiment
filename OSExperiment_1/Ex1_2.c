#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
void* thread_add();
void* thread_sub();
void wait();
void signal();
int value = 0;
int s = 0;
int main()
{
	pthread_t pid1, pid2;
	int res;
	res = pthread_create(&pid1, NULL, thread_add, NULL);
	if(res){
		printf("create failed!\n");
		return 0;
	}else printf("thread1 create success!\n");
	res = pthread_create(&pid2, NULL, thread_sub, NULL);
	if(res){	
		printf("create failed!\n");
		return 0;
	}else printf("thread2 create success!\n");
	pthread_join(pid1, NULL);
	pthread_join(pid2, NULL);
	printf("value = %d\n", value);
	return 0;
}
void* thread_add()
{
	wait();
	for(int i = 0; i < 100000; i++)
	{
		value +=100;
	}
	return 0;
}
void* thread_sub()
{
	for(int i = 0; i < 100000; i++)
	{
		value -= 100;
	}
	signal();
	return 0;
}
void wait()
{
	while(s<=0);
	s--;
}
void signal()
{
	s++;
}
