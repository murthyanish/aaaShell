#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<string.h>

int main(void)
{
	int size=0;
	FILE *in=NULL;
	char command[50];
	char temp[256];
	char c[100];
	strcpy(command,"ps -o pid");
	int a[100];
	
	int fd=open("ps.txt",O_CREAT| O_RDWR,0644);
	 int status=system("ps aux |grep defunc");
	if(WIFEXITED(status))
	{
		printf("Normal termination\n");
	}

	else if(WIFSIGNALED(status))	
	{
		printf("Abormal termination\n");
	}
}
