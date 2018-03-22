#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFLEN 1000
#define ARGMAX 100


void
runCmd(char **parsedInput){
	pid_t pid = fork();

	if(pid < 0){
		perror("Unable to run");
		return;
	}
	else if(pid == 0){
		//printf("exec %s\n", parsedInput[0]);
		if(execvp(parsedInput[0], parsedInput) < 0)
			printf("Command \"%s\" could not be executed\n", parsedInput[0]);
		exit(0);
	}
	else{
		wait(NULL);
		return;
	}
}


int
process(char *input, char **parsedInput){
	int i = -1;
	int processStatus = 0;
	char *userDefinedCommands[2];

	userDefinedCommands[0] = "exit";
	userDefinedCommands[1] = "cd";

	while(parsedInput[++i] = strsep(&input, " ")){
		if(i == 0){
			for (int i = 0; i < 2; ++i)
			{
				if(!strcmp(parsedInput[0], userDefinedCommands[i])){
					processStatus += ((i*4) + 3);
				}
			}
		}
		if((processStatus & 1) == 0){
			processStatus+=1;
		}
	}
	return processStatus;
}


int
main(int argc, char *argv[]){
	char *input = NULL, *parsedInput[ARGMAX];
	char *parsedPipe[ARGMAX];
	size_t size = 0;
	int n;
	int exit = 0;
	int processStatus;
	while(!exit){
		printf("|=");
		n = getline(&input, &size, stdin);
		input[n-1] = '\0';

		processStatus = process(input, parsedInput);

		//printf("processStatus = %d\n", processStatus);

		if ((processStatus & 1) == 1)
		{

			if ((processStatus & 2) == 2)
			{
				switch((processStatus-3)/4){
					case 0:
						exit = 1;
						break;
				}
			}
			else
				runCmd(parsedInput);
			
			
		}

	}
	return 0;
}