#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "stage.h"
#define INPUTLIMIT 512
#define CHILDERROR 127

static int interrupted = 0;
static int exited = 0;   // MARKER

static int interrupted = 0;
static void handlr(int signum);
int cd(char *path);

static void handlr(int signum)
{
    interrupted = 1;
}

int main(int argc, char *argv[]){
    struct sigaction sahint;
    sahint.sa_handler = handlr;
    sigaction(SIGINT, &sahint, NULL);
    int terminal = dup(1);
    int keyboard = dup(0);
    if (isatty(fileno(stdin)) || isatty(fileno(stdout))){  // MARKER
	printf("8-D ");
    }
    char orig[INPUTLIMIT + 1] = {'\0'};
    fgets(orig, INPUTLIMIT + 2, stdin);
    
    struct sigaction sahint;
    sahint.sa_handler = handlr;
    sigaction(SIGINT, &sahint, NULL);
    int terminal = dup(1);
    int keyboard = dup(0);
    if (isatty(fileno(stdin)) || isatty(fileno(stdout))){  // MARKER
	printf("8-D ");
    }
    char orig[INPUTLIMIT + 1] = {'\0'};
    fgets(orig, INPUTLIMIT + 2, stdin);
    exited = feof(stdin);     // MARKER
    if(!strcmp(orig, "end\n")){
	    return 1;
    }

    while (exited == 0){    // MARKER
	int prevPipe[2] = { 0 };
	int curPipe[2] = { 0 };
	struct stage **stages = parseline(orig);
	pid_t childAddress;
	int i;
	int output = -1;
	int input = -1;
	for(i = 0; stages[i] != NULL || interrupted == 0; i++){
	    if(stages[i + 1] != NULL){
		if(pipe(curPipe) == -1){
		    perror("Pipe Fails\n");
		}
	    }
	    /*sigproc mask will happen right before fork*/
	    sigset_t x;
	    sigemptyset (&x);
	    sigaddset(&x, SIGINT);
	    sigprocmask(SIG_BLOCK, &x, NULL);
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
		    output = open(stages[i]->output,
				  O_RDWR | O_TRUNC | O_CREAT, 0777);
		    dup2(output, STDOUT_FILENO);
		}
		sigprocmask(SIG_UNBLOCK, &x, NULL);
		execvp(stages[i]->argumentVar[0], stages[i]->argumentVar);
	    }else{
		int check;
		close(prevPipe[0]);
		close(prevPipe[1]);
		sigprocmask(SIG_UNBLOCK, &x, NULL);
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
    }
    return 1;
}   // MARKER

int cd(char *pth){
    if(pth == NULL){
	chdir(getenv("HOME"));
	return 1;
    }
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
        if(chdir(cwd) == -1){
            return 1;
        }
    }
    else{
        if (chdir(pth) == -1){
            perror("chdir");
            return 1;
        }
    }
    return 0;
}
