#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define BUFLEN 1000
#define ARGMAX 100

char *history[25] = {NULL};
int hstart = -1;
int hend = -1;
int full = 0;

/*int
histInit(){
	for (int i = 0; i < 25; i++){
		history[i] = malloc(BUFLEN);
	}
}*/

/*int histPrint(){
	for(int i = hstart; i+1)!=
}*/

/*int
histPush(char *input){
	if(hstart == -1){
		hstart = 0;
		hend = 0;
	}
	hend = (hend+1)%25;
	if(strcpy(history[hend], input))
		return -1;
	if(!full){
		if(hend == 0)	full = 1;
	}
	else if(hend == hstart+1){
		hstart++;
	}
	return 0;
}*/

//Function to run command sent in through parsedInput
void
runCmd(char **parsedInput){
	pid_t pid = fork();
	if(pid < 0){
		perror("Unable to run");
		return;
	}

	else if(pid == 0){
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
	}

	return processStatus;
}

void
runPipedCmd(char **parsedInput, int numPipes){

	numPipes++;
	printf("Entered runPipedCmd, numPipes = %d\n", numPipes);
	int pipefds[2*numPipes];

	for(int i = 0; i < (numPipes); i++){
		if(pipe(pipefds + i*2) < 0) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}

	printf("Pipes created\n");

	int m = -1;
	int n = 0;
	int pid;

	for (int i = 0; i < numPipes; ++i)
	{
		printf("Pipe %d\n", i);
		char *parsedPipe[ARGMAX] = {NULL};

		while(parsedInput[++m] && strcmp(parsedInput[m], "|") != 0){	//Parsing first command to be piped
			printf("copying %s\n", parsedInput[m]);
			parsedPipe[m-n] = parsedInput[m];
		}
		parsedPipe[m-n] = NULL;
		
		n = m+1;

		printf("Cmd: %s\n", parsedPipe[0]);

		pid = fork();
		if(pid == 0) {
			printf("Exec %d\n", i);
			//if not last command
			if(i != numPipes-1){

				if(dup2(pipefds[(i*2) + 1], 1) < 0){
					perror("dup2");
					exit(EXIT_FAILURE);
				}
			}

            //if not first command&& j!= 2*numPipes
			if(i != 0){
				if(dup2(pipefds[(i*2)-2], 0) < 0){
					perror("dup2");///j-2 0 j+1 1
					exit(EXIT_FAILURE);

				}
			}


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

    int status;

    while(wait(&status) > 0);

    /*for(int i = 0; i < numPipes; i++){
        wait(&status);
        printf("ended %d\n", i);
	}*/

	printf("End of runPipedCmd\n");

}

/*
void
runPipedCmd(char **parsedInput){	//Running of piped commands
	char *parsedPipe[ARGMAX] = {NULL};
	int i = -1;

	while(parsedInput[++i] && strcmp(parsedInput[i], "||") != 0){	//Parsing first command to be piped
		parsedPipe[i] = parsedInput[i];
	}
	parsedPipe[i] = NULL;

	runCmd(parsedPipe);

	if(parsedInput[i] != NULL)
		runPipedCmd(&parsedInput[i+1]);
}*/

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

		//checking the bit string
		if (processStatus & 1)	//valid command
		{
			if (processStatus & 2)	//user defined command
			{
				switch((processStatus-3)/4){
					case 0:		//exit case
						exit = 1;
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
			else if(processStatus & 16){
				runPipedCmd(parsedInput, (processStatus-16)/32);
			}
			else	//valid command which is not a user defined command
				runCmd(parsedInput);
		}
	}
	return 0;
}
