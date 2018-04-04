#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include "shell_main.h"



int main(int argc,char **argv){
	printf("____***Welcome to Rhu_Shell***____\n");
	printf("\t~~~*!Rhu the day I was born!*~~~\t\n");
	sleep(1);
	while(EXIT_SHELL){
		while((wpid = wait(&status)) > 0);
		printf("USER_%s~@%s@~: ",getuser(),getpwd());
		shell_start(shell_input());
	}
}







