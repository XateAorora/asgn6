#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "stage.h"
#define INPUTLIMIT 512

int main(int argc, char *argv[]){
    
    printf("line: ");
    char orig[INPUTLIMIT + 1] = {'\0'};
    fgets(orig, INPUTLIMIT + 2, stdin);
    
    struct stage **stages = parseline(orig);
    pid_t childAddress;
    int pipefd[2] = { 0 };
    pipe(pipefd);
    int i;
    printf("output: %s ;\n", stages[0]->output);
    int output;
    if(stages[0]->output != NULL){
	output = open(stages[0]->output, O_RDWR | O_TRUNC);
    }else{
	output = -1;
    }
    

    if((childAddress = fork()) == 0){
	/*if(output != -1){
	    dup2(output, STDOUT_FILENO);
	}
	close(pipefd[0]);*/
	execvp(stages[0]->argumentVar[0], stages[0]->argumentVar);
    }else{
	close(pipefd[1]);
	int check;
	waitpid(childAddress, &check, WCONTINUED);
	printf("Process Complete\n");
    }
    close(output);
    return 1;
}

int cd(char *pth){
    char path[INPUTLIMIT];
    char *NewLine = strchr(pth, '\n');
    if(NewLine) {
        *NewLine = '\0';
    }
    strcpy(path,pth);
    char cwd[INPUTLIMIT];
    if(pth[0] != '/'){ /*directory in cwd*/
        getcwd(cwd,sizeof(cwd));
        strcat(cwd,"/");
        strcat(cwd,path);
        int a = chdir(cwd);
        if (a == -1){
            return 1;
        }
    }
    else{
        int b=chdir(pth);
        if (b == -1){
            perror("chdir");
            return 1;
        }
    }
    return 0;
}
