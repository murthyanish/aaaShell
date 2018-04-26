
// ------------------------- BadShell -----------------------------
//	by Anish Murthy, Akhil Sundharam, Alekhya 
//	
//	Program flow:
//	-	main : 	executes the loop which gets input and sends it for processing, then running.
//		-	process :	processes the input and returns the separated input as well as the processStatus containing all runtime instructions for future commands
//		-	runProcessed :	runs the commands received using the instructions in processStatus
//			-	checks if process is a shell command or an executable command.
//			-	runShellCmd :	runs shell commands
//			-	


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <libgen.h>
#include <ctype.h>

#define BUFLEN 1000
#define ARGMAX 100

//ps code starts here
#define MAX_BUF 1024
#define INT_SIZE_BUF 6
#define PID_LIST_BLOCK 32
#define UP_TIME_SIZE 10


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

char *getUserName(int uid)
{
	struct passwd *pw = getpwuid(uid);
	if (pw)
	{
		return pw->pw_name;
	}

	return "";
}
 
void pidaux (char *buf)
{
	DIR *dirp;
	FILE *fp;
	struct dirent *entry;
	char path[MAX_BUF], read_buf[MAX_BUF],temp_buf[MAX_BUF];
	char uid_int_str[INT_SIZE_BUF]={0};
	char *line;
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
		fclose(fp);
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
		fclose(fp);
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
				for (int i = 0; i < 8; ++i)
				{
					getline(&line,&len,fp);	
				}
				sscanf(line,"Uid:    %s ",uid_int_str);
				for (int i = 0; i < 5; ++i)
				{
					getline(&line,&len,fp);	
				}
				sscanf(line,"VmSize:    %llu kB",&vmsize);
        		for (int i = 0; i < 4; ++i)
				{
					getline(&line,&len,fp);	
				}
				sscanf(line,"VmRSS:     %llu kB",&memory_rss);
				fclose(fp);
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
			int pid, ppid, pgrp, session, tty_nr, tpgid;
			unsigned long minflt, cminflt, majflt, cmajflt, utime, stime;
			unsigned long long starttime;
			long cutime, cstime, priority, nice, num_threads, itreavalue;
			
			sscanf(line,"%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld  %ld %llu",&pid,comm,&state,&ppid,&pgrp,&session,&tty_nr,&tpgid,&flags,&minflt,&cminflt,&majflt,&cmajflt,&utime,&stime,&cutime,&cstime,&priority,&nice,&num_threads,&itreavalue,&starttime);
			
			fclose(fp);

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
			}
			//For ps -x or ps -u root
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

//timer code
int get_no_of_spaces(char* cmd){
	int res = 0;
	int index = 0;
	char next = cmd[index];
	while (next != '\0')
	{
		if (next == ' ')
			res++;
		next = cmd[++index];
	}
	return res;
}

char msg[256];
void alarm_handler(int sig){
        printf("\n%s\n",msg);
}

void shell_timer(char* cmd){
	char s[256];
	//char msg[256];
	int i = 0;
	int no_arg = 0;
	no_arg = get_no_of_spaces(cmd) + 2;
	long int x=0;
	char* eptr;
	
	
	strcpy(s, cmd);
	char* token = strtok(s, " ");
	char* exec_argv[no_arg];
	
	while(token != NULL){
		exec_argv[i] = malloc(strlen(token) + 1);
		strcpy(exec_argv[i], token);
		//printf("i: %d  [%s]\n",i,exec_argv[i]);
		token = strtok(NULL, " ");
		i++;
	}
	
	exec_argv[i] = NULL;
	if(i==1){
		printf("Usage: timer [TIME] [MESSAGE]\n");
		return;}
	if(i==2){
		
		x = strtol(exec_argv[1],&eptr,10);
		if(errno == EINVAL){printf("~~Wrong timer input~~ \n");}
		strcpy(msg,"");
		strcat(msg,"~~~TIMER UP!~~~");
	}
	else{
		if(strcmp(exec_argv[1],"-msg")==0){
			
			x = strtol(exec_argv[2],&eptr,10);
			if(errno == EINVAL){printf("~~Wrong timer input~~ \n");}
			int tp =3;
			strcpy(msg,"");
			while(exec_argv[tp] != NULL){
				strcat(msg,exec_argv[tp++]);
				strcat(msg," ");
			}
		}
		//printf("here3\n");
	}
	
	alarm(x);
	return;
}


//Function to check if file is a regular file
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

//Function to search within a file for pattern and return locations where pattern is found
int searchFile(char *fname, char *str) {
	FILE *fp;
	int line_num = 1;
	char temp[512];
	
	if((fp = fopen(fname, "r")) == NULL) {
		return(-1);
	}

	while(fgets(temp, 512, fp) != NULL) {
		if((strstr(temp, str)) != NULL) {
			printf("file: %s\tline no: %d\t\nline content: ", fname, line_num);
			printf("%s", temp);
		}
		line_num++;
	}

	if(fp) {
		fclose(fp);
	}
   	return(0);
}

//Function recursively reads the directory for list of all files, and calls searchFile for each regular file
void readDir(char *directory, char *pattern, int recurse){
	//printf("in directory: %s\n", directory);
	struct dirent *de;  // Pointer for directory entry
 
    // opendir() returns a pointer of DIR type. 
    DIR *dr = opendir(directory);
 
    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory: %s\n", directory);
        return;
    }
    char* fullpath = malloc(sizeof(char)*100);;
    de = readdir(dr);
    de = readdir(dr);		//removing . & ..
    while ((de = readdir(dr)) != NULL){
    	strcpy(fullpath, "");
    	strcat(fullpath, directory);
    	strcat(fullpath, "/");
    	strcat(fullpath, de->d_name);
    	if(recurse && !is_regular_file(fullpath))
    		readDir(fullpath, pattern, recurse);
    	else if(is_regular_file(fullpath))
            searchFile(fullpath, pattern);
    }
 
    closedir(dr);   
}


//bfind function
//Searches for all occurances of a search pattern in the passed directory/file and returns the 
// line number and file name where it was found.
void
bfind(char ** parsedInput){
	//printf("In sgown\n");
	char **temp = parsedInput;
	int i;
	for(i=0;temp[i]!=NULL;i++);
	if(i<3){
		printf("Usage: bfind [-r] [PATH] [PATTERN]\n");
		return;
	}
	int recurse = 0;
	if(strcmp(parsedInput[1], "-r") == 0){
		recurse = 1;
		if(i!=4){
			printf("Usage: bfind [-r] [PATH] [PATTERN]\n");
			return;
		}
	}
			
	char *cwd = malloc(sizeof(char)*50);
	int readLoc = 1;
	if(recurse)
		readLoc = 2;

	//printf("recurse: %d, readLoc: %d\n", recurse, readLoc);
	if(strcmp(parsedInput[readLoc], "*") == 0){
		//printf("in *\n");
		if(!getcwd(cwd, 50)){
			perror("cwd");
			return;
		}
		//printf("cwd: %s\n", cwd);
		readDir(cwd, parsedInput[readLoc+1], recurse);
	}
	else if(!is_regular_file(parsedInput[readLoc])){
		readDir(parsedInput[readLoc], parsedInput[readLoc+1], recurse);
	}
	else
		searchFile(parsedInput[readLoc], parsedInput[readLoc+1]);
}

//Returns the input redirect and removes the input redirect code from command
char*
getRedirectIn(char ** input){
	int i;
	for(i = 0; strcmp(input[i], "<"); i++);
	if (input[++i] != NULL){
		//printf("redirectin\n");
		input[i-1] = NULL;
		char *output;
		strcpy(output, input[i]);
		if(input[i+1] != NULL){
			int j = 1;
			while(input[i+j] != NULL){
				input[i-2+j] = input[i+j];
				input[i+j] = NULL;
				j++;
			}
			input[i+j-2] = NULL;
		}
		/*if(output == NULL){
			printf("NULL\n");
		}*/
		//else printf("out: %s\n", output);
		return output;
	}
	else
		return NULL;
}

//Returns the output redirect and removes the output redirect code from command
char*
getRedirectOut(char ** input){
	int i;
	for(i = 0; strcmp(input[i], ">"); i++);
	if (input[++i] != NULL){
		//printf("out obtained %s\n", input[i-1]);
		input[i-1] = NULL;
		char *output;
		strcpy(output, input[i]);
		if(input[i+1] != NULL){
			//printf("notnull\n");
			int j = 1;
			while(input[i+j] != NULL){
				input[i+j-2] = input[i+j];
				input[i+j] = NULL;
				j++;
			}
			input[i+j-2] = NULL;
		}
		return output;
	}
	else
		return NULL;
}

//Check for redirect in symbol in input
int
hasRedirectIn(char **input){
	int i = 0;
	while(input[++i]!=NULL)
		if(strcmp(input[i], "<") == 0)
			return 1;
	return 0;
}

//Check for redirect out symbol in input
int
hasRedirectOut(char **input){
	int i = 0;
	while(input[++i]!=NULL)
		if(strcmp(input[i], ">") == 0)
			return 1;
	return 0;
}

//Function to run command sent in through parsedInput
//Includes redirects
void
runCmd(char **parsedInput, int redirects){
	pid_t pid = fork();
	if(pid < 0){
		perror("Unable to run");
		return;
	}

	else if(pid == 0){
		if ((redirects & 1) == 1)
		{
			int fd;
			if((fd = open(getRedirectIn(parsedInput), O_RDONLY)) < 0){
				perror("open");
				exit(EXIT_FAILURE);
			}
			if(dup2(fd, STDIN_FILENO) < 0){
				perror("dup2");
				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		if ((redirects & 2) == 2)
		{
			printf("out\n");
			int fd;
			if((fd = open(getRedirectOut(parsedInput), O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0){
				perror("open");
				exit(EXIT_FAILURE);
			}
			if(dup2(fd, STDOUT_FILENO) < 0){
				perror("dup2");
				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		if(execvp(parsedInput[0], parsedInput) < 0)
			printf("Command \"%s\" could not be executed\n", parsedInput[0]);
		exit(0);
	}

	else{
		wait(NULL);
		return;
	}
}


//Function processes input and returns the relevant bitstring containing run info.
//bit 0 = valid bit (can be run)
//bit 1 = UserDefinedCommand (tells shell to run the userdefined command rather than exec if set. Will simply run exec on input otherwise.)
//bit 3,4,5 =
//	if bit 1 is set to 1:
//		contains relevant user defined command id
//			000 - exit
//			001 - cd
//			010 - help
//			011 - bfind
//			100 - timer
//			101 - ps
//(eg. 1011 refers to a command to run help)
//(eg. 0001 refers to a command to run exec on input as is)
//bit 6 = Piping present flag
//bit 7,8,9 = Piping count
//bit 10 = Redirect in flag
//bit 11 = Redirect out flag
int
process(char *input, char **parsedInput){
	int i = -1;
	int processStatus = 0;

	char *userDefinedCommands[6];
	userDefinedCommands[0] = "exit";
	userDefinedCommands[1] = "cd";
	userDefinedCommands[2] = "help";
	userDefinedCommands[3] = "bfind";
	userDefinedCommands[4] = "timer";
	userDefinedCommands[5] = "ps";

	while(parsedInput[++i] = strsep(&input, " ")){
		if(i == 0){	//checking the first input substring
			for (int i = 0; i < 6; ++i){
				if(!strcmp(parsedInput[0], userDefinedCommands[i])){
					processStatus += ((i*4) + 3);
				}
			}
			if((processStatus & 1) == 0 && strcmp(parsedInput[i], "") != 0){
				processStatus += 1;
			}
		}
		if(!strcmp(parsedInput[i], "|")){	//checking for pipes in input
			processStatus = processStatus | 32;
			processStatus += 64;
		}

		if(!strcmp(parsedInput[i], "<")){
			processStatus = processStatus | 1024;
		}
		if(!strcmp(parsedInput[i], ">")){
			processStatus = processStatus | 2048;
		}
	}

	return processStatus;
}

//initialising pipes used in runPipedCmd
void
initPipes(int *pipefds, int numPipes){

	for(int i = 0; i < (numPipes); i++){
		if(pipe(&(pipefds[i*2])) < 0) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}
}

void
runPipedCmd(char **parsedInput, int numPipes, int redirects){

	numPipes++;
	//printf("Entered runPipedCmd, numPipes = %d\n", numPipes);
	int pipefds[2*(numPipes+1)];

	initPipes(pipefds, numPipes);

	//printf("Pipes created\n");

	int m = -1;
	int n = 0;
	int pid;
	int status;

	for (int i = 0; i < numPipes; ++i)
	{
		//printf("Pipe %d\n", i);
		char *parsedPipe[ARGMAX] = {NULL};

		while(parsedInput[++m] && strcmp(parsedInput[m], "|") != 0){	//Parsing first command to be piped
			//printf("copying %s\n", parsedInput[m]);
			parsedPipe[m-n] = parsedInput[m];
		}
		parsedPipe[m-n] = NULL;
		n = m+1;

		//printf("Cmd: %s\n", parsedPipe[0]);
		if (i != 0)
		{
			wait(&status);
		}
		pid = fork();
		if(pid == 0) {
			//printf("Exec %d\n", i);

			//if not first command
			//printf("if in %d\n", i);
			if ((redirects & 1) == 1 && hasRedirectIn(parsedPipe))
			{
				//printf("input redirect\n");
				int fd;
				if((fd = open(getRedirectIn(parsedPipe), O_RDONLY)) < 0){
					perror("open");
					exit(EXIT_FAILURE);
				}
				if(dup2(fd, STDIN_FILENO) < 0){
					perror("dup2");
					exit(EXIT_FAILURE);
				}
				//printf("%d\n", fd);
				close(fd);
			}
			else if(i != 0){
				write(pipefds[(i*2)-1], "\n\n", 2);
				//printf("Setting in of %d\n", i);
				if(dup2(pipefds[(i*2)-2], STDIN_FILENO) < 0){
					perror("dup2");///j-2 0 j+1 1
					exit(EXIT_FAILURE);

				}
			}

			if(((redirects & 2) == 2) && hasRedirectOut(parsedPipe)){
				//printf("output redirect\n");
				int fd;
				if((fd = open(getRedirectOut(parsedPipe), O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0){
					perror("open");
					exit(EXIT_FAILURE);
				}
				if(dup2(fd, STDOUT_FILENO) < 0){
					perror("dup2");
					exit(EXIT_FAILURE);
				}
				close(fd);
			}
			//if not last command
			if(i < numPipes-1){
				//printf("Setting out of %d\n", i);
				if(dup2(pipefds[(i*2)+1], STDOUT_FILENO) < 0){
					perror("dup2");
					exit(EXIT_FAILURE);
				}
			}
			//printf("end of out %d\n", i);



            for(int j = 0; j < 2*numPipes; j++){
                    close(pipefds[i]);
            }

            if( execvp(parsedPipe[0], parsedPipe) < 0 ){
                    perror(parsedPipe[0]);
                    exit(EXIT_FAILURE);
            }

		} else if(pid < 0){
			perror("error");
			exit(EXIT_FAILURE);
		}
    }
    /**Parent closes the pipes and wait for children*/

	for(int i = 0; i < 2 * numPipes; i++){
        close(pipefds[i]);
    }

    //while(wait(&status) > 0);

    wait(&status);

	//BUG: when using multiple command pipes with IO for each, program sometimes goes into an infinite wait.
	//	Bug does not appear when piping is removed and simply run
	//	Entering the required input does not make any change, main process simply ignores all input and continues to wait.

	//printf("End of runPipedCmd\n");

}


//status checkers
int
processIsValid(int ps){
	return (ps & 1);
}

int
processIsShellCmd(int ps){
	return (ps & 2);
}

int
processIsPiped(int ps){
	return (ps & 16);
}

int
processGetNumPipes(int ps){
	return (((ps%1024)-32)/64);
}

int
processShellCmdId(int ps){
	return ((ps-3)/4);
}

int processGetRedirects(int ps){
	return (ps/1024);
}


//runs user defined commands
void
processShellCmd(int processStatus, char **parsedInput, char *input, int *exit){
	switch(processShellCmdId(processStatus)){
		case 0:		//exit case
			*exit = 1;
			break;
		case 1:		//change directory case
			if(chdir(parsedInput[1]))
				perror("cd");
			else {
				char *cwd = malloc(sizeof(char)*100);
				getcwd(cwd, 100);
				setenv("PWD", cwd, 1);
				free(cwd);
			}
			break;
		case 2:		//help case
			printf("help:\nbadShell created by Anish M, Akhil S and Alekhya E\nMarch 2018\nCommands:\n\texit - quit the shell\n\tcd - change directory\n\thelp - Print help info\n\tsgown - search for substring in all files in specified directory\n");
			break;
		case 3:
			bfind(parsedInput);
			break;
		case 4:
			shell_timer(input);
			break;
		case 5:
			pidaux(input);
			break;
	}
}



void
runProcessed(int processStatus, char **parsedInput, char *input, int *exit){
		//checking the bit string
		if (processIsValid(processStatus))	//valid command
		{
			if (processIsShellCmd(processStatus))	//shell command
				processShellCmd(processStatus, parsedInput, input, exit);


			else if(processIsPiped(processStatus))		//command contains pipes
				runPipedCmd(parsedInput, processGetNumPipes(processStatus), processGetRedirects(processStatus));


			else	//valid command which is not a shell or piped command
				runCmd(parsedInput, processGetRedirects(processStatus));
		}
}



//Main function reads input and calls the relevant function or runs the relevant code.
int
main(int argc, char *argv[]){

	struct sigaction act;
	act.sa_handler = &alarm_handler;
	act.sa_flags = SA_RESTART;
	if(sigaction(SIGALRM,&act,NULL)<0)
		perror("sigaction");

	printf("--------- badShell ---------\n");

	char *input = NULL, *parsedInput[ARGMAX];	//input contains read input, parsedInput contains an array of the substrings of input.
	char mainInput[BUFLEN];
	//char *parsedPipe[ARGMAX];	//unimplemented, will be used for piped code processing/execution.
	size_t size = 0;	//min size of input string (used for getLine)
	int n;		//size of read input
	int exit = 0;	//exit flag
	int processStatus;	//processed input bit string

	while(!exit){
		printf("%s:%s |= ", getenv("USER"), getenv("PWD"));

		//getting input
		n = getline(&input, &size, stdin);
		input[n-1] = '\0';
		strcpy(mainInput, input);

		//processing input
		processStatus = process(input, parsedInput);

		runProcessed(processStatus, parsedInput, mainInput, &exit);

	}
	return 0;
}
