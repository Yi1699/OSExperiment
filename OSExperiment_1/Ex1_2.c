#include<unistd.h>
#include<pthread.h>
#include<stdio.h>
#include<semaphore.h>
void* thread_add();
void* thread_sub();
int value = 0;
sem_t s;
int main()
{
	sem_init(&s, 0, 1);
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
	for(int i = 0; i < 100000; i++)
	{
		sem_wait(&s);
		value +=100;
		sem_post(&s);
	}
	return 0;
}
void* thread_sub()
{
	for(int i = 0; i < 100000; i++)
	{
		sem_wait(&s);
		value -= 100;
		sem_post(&s);
	}
	return 0;
}
