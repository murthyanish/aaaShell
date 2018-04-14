#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
int main(void)
{
	int pid=fork();
	int status;
	if(pid==0)
	{
		exit(0);
	}
	sleep(50);

	pid=wait(&status);
	if(WIFEXITED(status))
	{
		printf("Process exited");
	}
	return 0;
}
