#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>
#include <pwd.h>
#include <unistd.h>

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
const char *getUserName(int uid)
{
  struct passwd *pw = getpwuid(uid);
  if (pw)
  {
    return pw->pw_name;
  }

  return "";
}
 
void pidaux()
{
	
	DIR *dp;
	DIR *dirp;
	char *user;
	struct dirent *entry;
	char uptime_str[10];
	char uid_int_str[6]={0};
	char path[1024];
	FILE *fp;
	unsigned long long memory_rss;
	unsigned long long vmsize;
	float memory_usage;
	float cpu_usage;
	char state;
	int pid;
	int ppid;
	char read_buf[1024];
	char *line;
	size_t len=0;
	dirp = opendir ("/proc/");
	if(dirp==NULL)
	{
		perror("Failed to open proc directory");
	}
	
	strcpy(path,"/proc/");
  	strcat(path,"uptime");
  	fp=fopen(path,"r");
	if(fp!=NULL)
  	{
		getline(&line,&len,fp);
		sscanf(line,"%s ",uptime_str);
  	}
	//printf("Uptime %s",line);
	long uptime=atof(uptime_str);
	long Hertz=sysconf(_SC_CLK_TCK);
	
	strcpy(path,"/proc/");
  	strcat(path,"meminfo");

  	fp=fopen(path,"r");
  	unsigned long long total_memory;
  	if(fp!=NULL)
  	{
		getline(&line,&len,fp);
		sscanf(line,"Total memory:        %llu kB",&total_memory);
 	 }	
	//printf("Memory%s",line);
	while ((entry = readdir (dirp)) != NULL)
  	{
		//printf("entered while lopp");
		if (check_if_number (entry->d_name))
    		{
			strcpy(path,"/proc/");
			strcat(path,entry->d_name);
			//printf("hi\n");
			
			strcat(path,"/status");
			//printf("%s\n",path);
			
			fp=fopen(path,"r");
			
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
			unsigned long utime,stime;
			unsigned long long starttime;
			long cutime;
			long cstime;
			int pgrp,session,tty_nr,tpgid;
			unsigned long minflt,cminflt,majflt,cmajflt;
			char comm[10];
			unsigned int flags;
			long priority;
			long nice,num_threads,itreavalue;
			 memory_usage=100*(memory_rss/total_memory);
			//printf("Memory usage %d\n",memory_usage);
			sscanf(line,"%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld  %ld %llu",&pid,comm,&state,&ppid,&pgrp,&session,&tty_nr,&tpgid,&flags,&minflt,&cminflt,&majflt,&cmajflt,&utime,&stime,&cutime,&cstime,&priority,&nice,&num_threads,&itreavalue,&starttime);
			//sscanf(line,"%d %c %d %lu %lu %ld %ld %llu",&pid, &state, &ppid,&utime,&stime,&cutime,&cstime,&starttime);			
			unsigned long total_time=utime+stime;
			total_time=total_time+(unsigned long)cutime+(unsigned long)cstime;
			//printf("Total time %d\n",total_time);
			float seconds=uptime-(starttime/Hertz);
			//printf("SEconds %d\n",seconds);
			//printf("Total time %d\n",total_time);
			//printf("SEconds %d\n",seconds);
			//printf("Hertz%d\n",Hertz);
			float val=(total_time/Hertz);
			//printf("Value%d",val);
			float result=val/seconds;
			//printf("REsult%f",result);
			//printf("CPU---------%0.1f\n",val);
			cpu_usage=100*((total_time/Hertz)/seconds);
			if(isnan(cpu_usage))
			{
				cpu_usage=0.0;
			}
			if(isnan(memory_usage))
			{
				memory_usage=0.0;
			}
			//printf("CPUusage %d\n",cpu_usage);
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
			//printf("%s\n",userName);
		     	if(strlen(userName)<10)
		      	{
				user=userName;	
		      	}
		      	else
		      	{
				user=uid_int_str;
		      	}
			//printf("User here %s\n",user);
			//printf("Before fprintf");
			//fprintf(stdout,"%s %s %0.1f %0.1f %llu %llu %c %s\n",user,entry->d_name,cpu_usage,memory_usage,vmsize,memory_rss,state,read_buf);
			//printf("%s\t %s\t %f ",user , entry->d_name,val);
			//printf("State:-%c\n",state);
			fprintf(stdout,"%s\t %s\t %0.1f\t %0.1f\t %llu\t %llu\t %c\t %s \n",user,entry->d_name,cpu_usage,memory_usage,vmsize,memory_rss,state,read_buf);
	
		}
	}
	
	
}
int main (int argc, char *argv[])
{
  pidaux();	
  return 0;
}
