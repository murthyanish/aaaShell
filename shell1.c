
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<time.h>
#include<fcntl.h>
#include<dirent.h>
int split(char* str, char** parsed)
{

    int i=0;

    while(1) {
        parsed[i++] = strsep(&str, " ");
        if (parsed[i-1] == NULL)
 
           break;
	if (strlen(parsed[i-1]) == 0)
            i--;
	}
	pid_t pid = fork(); 

     if (pid == 0) {
       execvp(parsed[0], parsed);
        exit(0);
    } 
	return 1;
}

int read_input(char* str)
{
    char* buf, *buf_temp;
    buf = readline("\n>>> ");

     int len = strlen(buf);
	if( buf[len-1] == '\\')
	{
	    while(buf[len-1] == '\\')
		{
			buf_temp = readline(">");
			buf[len-2]='\0';
			strcat(buf,buf_temp);
			len = strlen(buf);
		}
	}
	
    strcpy(str, buf);
	printf("%s\n",buf);
	
	char *args[]={str,NULL};
	
    return 0;
}
int check_file()
{
	struct stat sb;
	struct stat *buf;
	DIR *dp=NULL;
	struct dirent *dptr=NULL;
	if((dp=opendir("/home/alekhya/bigdata"))<0)
	{
		perror("opendir");
	}
	while((dptr=readdir(dp))!=NULL)
	{
		int ret=chdir("/home/alekhya/bigdata");
		stat("/home/alekhya/bigdata",&sb);
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
int main(void)
{
	char input_str[1000];
    	char  *tokens[100];

    	int typeof_exec = 0;
	read_input(input_str);
	split(input_str,tokens);
	
	return 0;
	
}
