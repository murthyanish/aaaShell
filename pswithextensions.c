#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>
#include <pwd.h>
#include <unistd.h>
char* buf, *buf_temp; 
int check_if_number (char *str)
{
  int i;
  for (i=0; str[i] != '\0'; i++)
  {
    if (!isdigit (str[i]))
    {
      return 0;
    }
  }
  return 1;
}
int split(char* str, char** parsed)
{

    int i=0;

    while(1) 
   {
        parsed[i++] = strsep(&str, " ");
	
        if (parsed[i-1] == NULL)
 
           break;
	if (strlen(parsed[i-1]) == 0)
            i--;
   }
	pid_t pid = fork(); 

     if (pid == 0) 
    {
       
        exit(0);
    } 
	return 1;
}

int read_input(char* str)
{
    
    buf = readline("\n>>> ");
    int len = strlen(buf);
    // printf("%s\n",buf);
    char *args[]={str,NULL};
	return 0;
}


#define MAX_BUF 1024
#define INT_SIZE_BUF 6
#define PID_LIST_BLOCK 32
#define UP_TIME_SIZE 10
const char *getUserName(int uid)
{
  struct passwd *pw = getpwuid(uid);
  if (pw)
  {
    return pw->pw_name;
  }

  return "";
}
 
void pidaux (char *ch1)
{
  DIR *dirp;
  FILE *fp;
  struct dirent *entry;
  char path[MAX_BUF], read_buf[MAX_BUF],temp_buf[MAX_BUF];
  char uid_int_str[INT_SIZE_BUF]={0},*line;
  char uptime_str[UP_TIME_SIZE];
  char *user,*command;
  size_t len=0;
  dirp = opendir ("/proc/");
  if (dirp == NULL)
  {
         perror ("Fail");
         exit(0);
  } 
  strcpy(path,"/proc/");
  strcat(path,"uptime");
 
  fp=fopen(path,"r");
  if(fp!=NULL)
  {
	getline(&line,&len,fp);
	sscanf(line,"%s ",uptime_str);
  }
  long uptime=atof(uptime_str);
  long Hertz=sysconf(_SC_CLK_TCK); 
  strcpy(path,"/proc/");
  strcat(path,"meminfo");

  fp=fopen(path,"r");
  unsigned long long total_memory;
  if(fp!=NULL)
  {
	getline(&line,&len,fp);
	sscanf(line,"MemTotal:        %llu kB",&total_memory);
  }	

  while ((entry = readdir (dirp)) != NULL)
  {
    if (check_if_number (entry->d_name))
    {
	strcpy(path,"/proc/");
	strcat(path,entry->d_name);
	strcat(path,"/status");
	unsigned long long memory_rss;
	fp=fopen(path,"r");
	unsigned long long vmsize;

	if(fp!=NULL)
	{
		vmsize=0;
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		sscanf(line,"Uid:    %s ",uid_int_str);
		getline(&line,&len,fp);
                getline(&line,&len,fp);
                getline(&line,&len,fp);
		getline(&line,&len,fp);
                getline(&line,&len,fp);
		sscanf(line,"VmSize:    %llu kB",&vmsize);
                getline(&line,&len,fp);
                getline(&line,&len,fp);
                getline(&line,&len,fp);
		getline(&line,&len,fp);
		sscanf(line,"VmRSS:     %llu kB",&memory_rss);
		
	}
	else
	{
		fprintf(stdout,"FP is NULL\n");
	}
	float memory_usage=100*memory_rss/total_memory;
	strcpy(path,"/proc/");
	strcat(path,entry->d_name);
	strcat(path,"/stat");
	fp=fopen(path,"r");
	getline(&line,&len,fp);
	char comm[10],state;
	unsigned int flags;
	int pid,ppid,pgrp,session,tty_nr,tpgid;
	unsigned long minflt,cminflt,majflt,cmajflt,utime,stime;
	unsigned long long starttime;
	long cutime,cstime,priority,nice,num_threads,itreavalue;
	sscanf(line,"%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld  %ld %llu",&pid,comm,&state,&ppid,&pgrp,&session,&tty_nr,&tpgid,&flags,&minflt,&cminflt,&majflt,&cmajflt,&utime,&stime,&cutime,&cstime,&priority,&nice,&num_threads,&itreavalue,&starttime);
	unsigned long total_time=utime+stime;
	total_time=total_time+(unsigned long)cutime+(unsigned long)cstime;
	float seconds=uptime-(starttime/Hertz);
	float cpu_usage=100*((total_time/Hertz)/seconds);
	if(isnan(cpu_usage))
	{
		cpu_usage=0.0;
	}
	if(isnan(memory_usage))
	{
		memory_usage=0.0;
	}
	strcpy (path, "/proc/");
	strcat (path, entry->d_name);
	strcat (path, "/comm");
	 
      	fp = fopen (path, "r");
      	if (fp != NULL)
      	{
        	fscanf (fp, "%s", read_buf);
		fclose(fp);
      	}
      	char *userName=getUserName(atoi(uid_int_str));
     	if(strlen(userName)<9)
      	{
		user=userName;	
      	}
      	else
      	{
		user=uid_int_str;
      	}
	if(strcmp(buf,"ps")==0)
	{
      	fprintf(stdout,"%s %s %0.1f %0.1f %llu %llu %c %s\n",user,entry->d_name,cpu_usage,memory_usage,vmsize,memory_rss,state,read_buf);
	//For ps -x or ps -u root
	
	
	}
	if(strcmp(buf,"ps -x")==0)
	{
		if((strcmp(user,"root")==0))
		{
		
			fprintf(stdout,"%s %s %0.1f %0.1f %llu %llu %c %s\n",user,entry->d_name,cpu_usage,memory_usage,vmsize,memory_rss,state,read_buf);
		}
		//printf("String successfully compared");
	}
	//For ps -e or ps -A
	if(strcmp(buf,"ps -e")==0)
	{
		if(state=='R')
		{
			fprintf(stdout,"%s %s %0.1f %0.1f %llu %llu %c %s\n",user,entry->d_name,cpu_usage,memory_usage,vmsize,memory_rss,state,read_buf);
			//printf("Active processes implementatio");
			printf("%s",user);
		}
	}
	//Display user running processes - all the processes owned by me
	//ps -x
	//implementation for ps -fp pid
	if(strcmp(buf,"ps -fp 1")==0)
	{
	
		if(strcmp(entry->d_name,"1")==0)
		{
			printf("---------------------------Hey requested processes-------------------------------\n");
			fprintf(stdout,"%s %s %0.1f %0.1f %llu %llu %c %s\n",user,entry->d_name,cpu_usage,memory_usage,vmsize,memory_rss,state,read_buf);	
		}
	}
	//do make selection using pid list
	//ps -fp pid1,pid2
	int pos=0;
	int length=12;
	int c=0;
	char sub[1000];
	while(c<length)
	{
		sub[c]=buf[pos+c];
		c++;
		
	}
	sub[c]='\0';
	//printf("Substring is here");
	//printf("%s\n",sub);
	int x;
	if(strcmp(sub,"ps -f --ppid")==0)
	{
		//printf("Entered string comparison");
		char *p=strrchr(buf,' ');
		if(p && *(p+1))
		{
			//printf("---------------------------------------------------------------Parent pid is here %s\n",ppid);
			sscanf((p+1),"%d",&x);
			if(x==ppid)
			{
			
				fprintf(stdout,"%s %s %0.1f %0.1f %llu %llu %c %s\n",user,entry->d_name,cpu_usage,memory_usage,vmsize,memory_rss,state,read_buf);
			
		
			}
		}
	}
	
     	 
    }
  } 
  closedir (dirp);
}
 
int main (int argc, char *argv[])
{
	char input_str[1000];
    	char  *tokens[100];

    	int typeof_exec = 0;
	read_input(input_str);
	split(input_str,tokens);
  
  pidaux(buf);	
  return 0;
}
