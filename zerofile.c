//Remove empty files  in a given directory
//Directory name is taken from the user
#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<time.h>
#include<dirent.h>
int main(int argc,char *argv[])
{
	struct stat sb;
	struct stat *buf;
	DIR *dp=NULL;
	struct dirent *dptr=NULL;
	char s[100];
	scanf("%s",&s);
	if((dp=opendir(s))<0)
	{
		perror("opendir");
	}
	while((dptr=readdir(dp))!=NULL)
	{
		int ret=chdir(s);
		stat(s,&sb);
		if(sb.st_size==0)
		{
			remove(dptr->d_name);
		}
		
		if(sb.st_nlink==2)
		{
				remove(dptr->d_name);
		}	
	}
	return 0;
	
}
