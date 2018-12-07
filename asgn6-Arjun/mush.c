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
    int terminal = dup(1);
    int keyboard = dup(0);
    if (isatty(fileno(stdin)) || isatty(fileno(stdout))){
	    printf("8-D ");
    }
    char orig[INPUTLIMIT + 1] = {'\0'};
    fgets(orig, INPUTLIMIT + 2, stdin);
    if(!strcmp(orig, "end\n")){
	return 1;
    }
    
    do{
	int prevPipe[2] = { 0 };
	int curPipe[2] = { 0 };
	struct stage **stages = parseline(orig);
	pid_t childAddress;
	int i;
	int output = -1;
	int input = -1;
	for(i = 0; stages[i] != NULL; i++){
	    if(stages[i + 1] != NULL){
		if(pipe(curPipe) == -1){
		    perror("Pipe Fails\n");
		}
	    }
	/*sigproc mask will happen right before fork*/
	sigset_t x;
        sigemptyset (&x);
        sigaddset(&x, SIGINT);
        sigprocmask(SIG_BLOCK, &x, NULL)
	    if((childAddress = fork()) == 0){
		if(i){
		    printf("reading back\n");
		    dup2(prevPipe[0], STDIN_FILENO);
		    close(prevPipe[0]);
		    close(prevPipe[1]);
		}else if(stages[i]->input != NULL){
		    input = open(stages[i]->input, O_RDONLY);
		    dup2(input, STDIN_FILENO);
		}

		if(stages[i + 1] != NULL){
		    printf("reading out\n");
		    close(curPipe[0]);
		    dup2(curPipe[1], STDOUT_FILENO);
		    close(curPipe[1]);
		}else{
		    output = open(stages[i]->output, O_RDWR | O_TRUNC | O_CREAT, 0777);
		    dup2(output, STDOUT_FILENO);
		}
	   	sigprocmask(SIG_UNBLOCK, &x, NULL)
		execvp(stages[i]->argumentVar[0], stages[i]->argumentVar);
	    }else{
		int check;
		close(prevPipe[0]);
		close(prevPipe[1]);
		waitpid(childAddress, &check, WCONTINUED);
		printf("Process Complete\n");
		if(pipe(prevPipe) == -1){
		    perror("Pipe Fails\n");
		}
		prevPipe[0] = curPipe[0];
		prevPipe[1] = curPipe[1];
	    }
	}
	close(prevPipe[0]);
	close(prevPipe[1]);
	close(curPipe[0]);
	close(curPipe[1]);
	
	dup2(keyboard, STDIN_FILENO);
	dup2(terminal, STDOUT_FILENO);
	printf("8-D ");
	fgets(orig, INPUTLIMIT + 2, stdin);
    }while(strcmp(orig, "end\n"));
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
