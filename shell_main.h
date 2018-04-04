

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>

#define	start_index	0
#define alias_max	10
#define history_max	10


//Alias variables
struct ALIAS{
	char* name;
	char* value;
	};
struct ALIAS alias_vals[alias_max];
int alias_curr = -1;

// History variables
char *history_vals[history_max];
int hist_begin = -1;
int hist_curr = -1;


//Others
int EXIT_SHELL = 1;
pid_t wpid;
int status = 0;



char* getpwd(){
	//char a[256];
	getenv("PWD");
}



int is_alias(char* cmd){
	if(alias_curr == -1){return -1;}
	for(int i = 0; i<= alias_curr;i++){
		if(strcmp(cmd,alias_vals[i].name) == 0){
			return i;
		}
	}
	return -1;
}

//CUSTOM FUCNTIONS - for shell specific commands
 //cd = returns 1
 //alias = returns 2
 //aliased func = return -2
 //history = returns 3
int is_custom(char* cmd){
	char* token = strtok(cmd," ");
	if(strcmp("cd",token) == 0){
		return 1;
	}
	if(strcmp("alias",token) == 0){
		return 2;
	}
	if(strcmp("history",token) == 0){
		return 3;
	}
	if(is_alias(cmd) != -1){
		return -2;
	}
	else{
		return -1;
	}
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




//Change DIR (CD)
void shell_cd(char* cmd){ //change dir -CD
	char s[256];
	char first[64];
	int i = 0;
	int no_arg = 0;
	no_arg = get_no_of_spaces(cmd) + 2;
	//int cus = 0;
	
		
	//
	strcpy(s, cmd);
	char* token = strtok(s, " ");
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
	int r = chdir(exec_argv[1]);
	if(r == -1){perror("chdir");return;}
	else{
		printf("CD SUCCESSFUL\n");
		//printf(" pwd : %s\n", getenv("PWD"));
		char buf[256];
		char* new_path;
		new_path = getcwd(buf,256);
		printf(" pwd : %s\n", new_path);
		setenv("PWD",new_path,1);
	}
	return;
}


//Alias
void shell_alias(char* cmd){
	char s[256];
	char first[64];
	int i = 0;
	int no_arg = 0;
	no_arg = get_no_of_spaces(cmd) + 2;
	int index;
	
	
	//printf("%s\n",exec_argv[i-1]);
	
	if(strcmp("alias",cmd) == 0){
		printf("~~Aliases~~\n");
		if(alias_curr == -1){printf("***NO Aliases set\n");}
		else{
			for(int k=0;k<=alias_curr;k++){
				printf("[%s] : [%s]\n",alias_vals[k].name,alias_vals[k].value);
			}
		}
	}
	else{
		strcpy(s, cmd);
		char* token = strtok(s, " ");
		char* exec_argv[no_arg];
	
		exec_argv[i] = malloc(strlen(token) + 1);
		strcpy(exec_argv[i], token);
		token = strtok(NULL, "\0");
		i++;
	
		exec_argv[i] = malloc(strlen(token)+1);
		strcpy(exec_argv[i],token);
		i++;
		exec_argv[i] = NULL;
		char* str = malloc(sizeof(char) * (strlen(exec_argv[1])+1));
		strcpy(str,exec_argv[1]);
		char* n; char* v;
		n = strtok(str,"=");
		v = strtok(NULL,"\0");
		if((index = is_alias(n)) == -1){
			if((alias_curr + 1) < alias_max){
				alias_curr++;
				alias_vals[alias_curr].name = n;
				alias_vals[alias_curr].value = v;
			}
			else{
				printf("MAX ALIAS REACHED ~~ no can do ~~\n");
			}
		}
		else{
			alias_vals[index].value = v;
		} 
	}
	
}


void shell_exec(char* cmd){
	char s[256];
	char first[64];
	int i = 0;
	int no_arg = 0;
	no_arg = get_no_of_spaces(cmd) + 2;
	//int cus = 0;
	
		
	//
	strcpy(s, cmd);
	char* token = strtok(s, " ");
	char* exec_argv[no_arg];
	//printf("%s\n",cmd);
	
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
			//printf("In child arg1: %s \n",exec_argv[0]);
			//execlp(token,token,NULL);
			execvp(exec_argv[0],exec_argv);
			exit(0);
	}
	for(int k=0;k<no_arg;k++){
		free(exec_argv[k]);
	}
	return;
}


char* getuser(){
	//shell_exec("whoami");
	//char a[50];
	getenv("USER");
}

char* shell_input(){
	size_t len;
	ssize_t read;
	char* cmd = NULL;
	read = getline(&cmd, &len,stdin);
	int cus = 0;
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


//History
void shell_add_history(char *cmd){
	if(history_vals[(hist_curr+1)%(history_max)] != 0){free( history_vals[(hist_curr+1)%(history_max)] );} 
	history_vals[(hist_curr+1)%(history_max)] = (char*)malloc(sizeof(char) * (strlen(cmd)+1));
	strcpy(history_vals[(hist_curr+1)%(history_max)],cmd);
	
	if(hist_begin == -1 || ((hist_curr+1)%history_max)==hist_curr ){hist_begin++;}
	hist_curr = (hist_curr+1)%history_max;	
	return;
}

void shell_history(){
	if(hist_begin == -1){
		printf("~~Shell is starving\n\t, execute more cmds pls~~\n");
		return;
	}
	
	else{
		if(hist_curr == hist_begin){printf("~[%s]\n",history_vals[hist_begin]);}
		else{
			int ct = 1;
			for(int i = hist_begin; i != hist_curr; i = (i+1)%history_max){
				printf("#%d~[%s]\n",ct++,history_vals[i]);
			}
			printf("#%d~[%s]\n",ct++,history_vals[hist_curr]);
		}
	}
	return;
}



void shell_start(char *cmd){
	//clear();
	/*if(strcmp("!q",cmd) == 0){
		printf("\n!q typed\n");
		EXIT_SHELL = 0;
		return;
	}
	*/
	int cus;
	
	if(strlen(cmd) == 1){
		//printf("no input\n");
		return;
	}
	else{
		strtok(cmd,"\n");
		if(strcmp("!q",cmd) == 0){
			printf("\t~EXITING Rhu_Shell~\n");
			EXIT_SHELL = 0;
			return;
		}
		char s[256];
		strcpy(s,cmd);
		if((cus = is_custom(s)) != -1){
			if(cus == 1){
				// CD
				shell_cd(cmd);
				return;
			}
			if(cus == 2){
				//Alias
				shell_alias(cmd);
			}
			if(cus == -2){
				//aliased functions
				shell_exec(alias_vals[is_alias(cmd)].value);
			}
			if(cus == 3){
				//History
				shell_history();
			}
		}
		else{
			shell_exec(cmd);
		}
		shell_add_history(cmd);
	}

	return;
}

