
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFLEN 1000
#define ARGMAX 100

//#include "badShell.h"


char *getRedirectIn(char ** input){
	int i;
	for(i = 0; strcmp(input[i], "<"); i++);
	if (input[++i] != NULL){
		printf("redirectin\n");
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
		if(output == NULL){
			printf("NULL\n");
		}
		else printf("out: %s\n", output);
		return output;
	}
	else
		return NULL;
}

char *getRedirectOut(char ** input){
	int i;
	for(i = 0; strcmp(input[i], ">"); i++);
	if (input[++i] != NULL){
		printf("out obtained %s\n", input[i-1]);
		input[i-1] = NULL;
		char *output;
		strcpy(output, input[i]);
		if(input[i+1] != NULL){
			printf("notnull\n");
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

int
hasRedirectIn(char **input){
	int i = 0;
	while(input[++i]!=NULL)
		if(strcmp(input[i], "<") == 0)
			return 1;
	return 0;
}

int
hasRedirectOut(char **input){
	int i = 0;
	while(input[++i]!=NULL)
		if(strcmp(input[i], ">") == 0)
			return 1;
	return 0;
}

//Function to run command sent in through parsedInput
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
//bit 3&4 =
//	if bit 1 is set to 1:
//		contains relevant user defined command id
//			00 - exit
//			01 - cd
//			10 - help
//(eg. 1011 refers to a command to run help)
//(eg. 0001 refers to a command to run exec on input as is)
int
process(char *input, char **parsedInput){
	int i = -1;
	int processStatus = 0;

	char *userDefinedCommands[3];
	userDefinedCommands[0] = "exit";
	userDefinedCommands[1] = "cd";
	userDefinedCommands[2] = "help";

	while(parsedInput[++i] = strsep(&input, " ")){
		if(i == 0){	//checking the first input substring
			for (int i = 0; i < 3; ++i){
				if(!strcmp(parsedInput[0], userDefinedCommands[i])){
					processStatus += ((i*4) + 3);
				}
			}
			if((processStatus & 1) == 0 && strcmp(parsedInput[i], "") != 0){
				processStatus += 1;
			}
		}
		if(!strcmp(parsedInput[i], "|")){	//checking for pipes in input
			processStatus = processStatus | 16;
			processStatus += 32;
		}

		if(!strcmp(parsedInput[i], "<")){
			processStatus = processStatus | 512;
		}
		if(!strcmp(parsedInput[i], ">")){
			processStatus = processStatus | 1024;
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
				write(pipefds[(i*2)-1], "\n", 1);
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

	//BUG: when using multiple command pipes with IO for each, program goes into an infinite wait.
	//	Bug does not appear when piping is removed and simply run
	//	Entering the required input does not make any change, main process simply ignored all input and continues to wait.

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
	return (((ps%512)-16)/32);
}

int
processShellCmdId(int ps){
	return ((ps-3)/4);
}

int processGetRedirects(int ps){
	return (ps/512);
}


//runs user defined commands
void
processShellCmd(int processStatus, char **parsedInput, int *exit){
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
			printf("help:\nbadShell created by Anish M, Akhil S and Alekhya E\nMarch 2018\nCommands:\n\texit - quit the shell\n\tcd - change directory\n\thelp - Print help info\n");
	}
}



void
runProcessed(int processStatus, char **parsedInput, int *exit){
		//checking the bit string
		if (processIsValid(processStatus))	//valid command
		{
			if (processIsShellCmd(processStatus))	//shell command
				processShellCmd(processStatus, parsedInput, exit);


			else if(processIsPiped(processStatus))		//command contains pipes
				runPipedCmd(parsedInput, processGetNumPipes(processStatus), processGetRedirects(processStatus));


			else	//valid command which is not a shell or piped command
				runCmd(parsedInput, processGetRedirects(processStatus));
		}
}



//Main function reads input and calls the relevant function or runs the relevant code.
int
main(int argc, char *argv[]){
	printf("--------- badShell ---------\n");

	char *input = NULL, *parsedInput[ARGMAX];	//input contains read input, parsedInput contains an array of the substrings of input.
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

		//processing input
		processStatus = process(input, parsedInput);

		runProcessed(processStatus, parsedInput, &exit);

	}
	return 0;
}
