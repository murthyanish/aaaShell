

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>

int EXIT_SHELL = 1;
pid_t wpid;
int status = 0;


int is_custom(char* cmd){
	
}

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

void shell_exec(char* cmd){
	char s[256];
	char first[64];
	int i = 0;
	int no_arg = 0;
	no_arg = get_no_of_spaces(cmd) + 2;
	
		
	//
	strcpy(s, cmd);
	char* token = strtok(s, " ");
	strcpy(first,token);
	char* exec_argv[no_arg];
	//printf("%s\n",first);
	
	//
	while(token != NULL){
		exec_argv[i] = malloc(strlen(token) + 1);
		strcpy(exec_argv[i], token);
		//printf("i: %d  [%s]\n",i,exec_argv[i]);
		token = strtok(NULL, " ");
		i++;
	}
	exec_argv[i] = NULL;
		
	pid_t child = fork();
	if(child == 0){
		//printf("In child\n");
		//execlp(token,token,NULL);
		execvp(first,exec_argv);
		exit(0);
	}
	
	
	while (token) {
		token = strtok(NULL, " ");
	}
}


char* getuser(){
	//shell_exec("whoami");
	char a[50];
	strcpy(a,getenv("USER"));
}

char* shell_input(){
	size_t len;
	ssize_t read;
	char* cmd = NULL;
	read = getline(&cmd, &len,stdin);
	if(read == -1){
		perror("getline");
	}
	else{
		//printf("\ninput:%s\n",cmd);
		//printf("%d\n",read);
		//printf("%c",cmd[3]);
		return cmd;
	}
}

void shell_start(char *cmd){
	//clear();
	/*if(strcmp("!q",cmd) == 0){
		printf("\n!q typed\n");
		EXIT_SHELL = 0;
		return;
	}
	*/
	
	if(strlen(cmd) == 1){
		printf("no input\n");
		return;
	}
	else{
		strtok(cmd,"\n");
		if(strcmp("!q",cmd) == 0){
			printf("\t~EXITING Rhu_Shell~\n");
			EXIT_SHELL = 0;
			return;
		}
		shell_exec(cmd);
	}

	return;
}

