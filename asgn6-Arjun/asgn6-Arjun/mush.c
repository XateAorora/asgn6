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

static void handlr(int signum);
int cd(char *path);

int fileRead(char *output, int input);

int fileRead(char *output, int input){
    char wordHold[INPUTLIMIT + 1] = {'\0'};
    int i = 0;
    for(; i < INPUTLIMIT ; i++){
	read(input, &(wordHold[i]), sizeof(char));
	if(wordHold[i] == '\n' || wordHold[i] == '\0' || wordHold[i] == EOF){
	    output = wordHold;
	    return 1;
	}
    }
    return -1;
}

static void handlr(int signum)
{
    interrupted = 1;
}

int main(int argc, char *argv[]){
    int filePtr = -1;
    if(argc > 1){
	filePtr = open(argv[1], O_RDONLY);
    }
    struct sigaction sahint;
    sahint.sa_handler = handlr;
    sigaction(SIGINT, &sahint, NULL);
    int terminal = dup(1);
    int keyboard = dup(0);
    if ((isatty(fileno(stdin)) || isatty(fileno(stdout))) && (argc == 1)){
	    printf("8-D ");
    }
    char orig[INPUTLIMIT + 1] = {'\0'};
    if(argc == 1){
	(fgets(orig, INPUTLIMIT + 2, stdin));
    }else{
	fileRead(orig, filePtr);
    }
    exited = feof(stdin);
    if(!strcmp(orig, "end\n")){
	return 1;
    }

    
    sigset_t x;
    sigemptyset (&x);
    sigaddset(&x, SIGINT);
    
    while(exited == 0){
	int prevPipe[2] = { 0 };
	int curPipe[2] = { 0 };
	struct stage **stages = parseline(orig);
	if(stages != NULL){
	    if(!strcmp(stages[0]->argumentVar[0], "cd")){
		char *path = NULL;
		if(stages[0]->argumentCount > 1){
		    path = stages[0]->argumentVar[1];
		}
		cd(path);
	    }
	    pid_t childAddress;
	    int i;
	    int output = -1;
	    int input = -1;
	    sigprocmask(SIG_BLOCK, &x, NULL);
	    for(i = 0; stages[i] != NULL && interrupted == 0; i++){
		if(stages[i + 1] != NULL){
		    if(pipe(curPipe) == -1){
			perror("Pipe Fails\n");
		    }
		}
		/*sigproc mask will happen right before fork*/
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
		    exit(CHILDERROR);
		}else{
		    sigprocmask(SIG_UNBLOCK, &x, NULL);
		    int check = -1;
		    close(prevPipe[0]);
		    close(prevPipe[1]);
		    waitpid(childAddress, &check, WCONTINUED);
		    if(check == 127){
			perror("Child Process Failed!");
			exit(127);
		    }
		    printf("Process Complete\n");
		    if(pipe(prevPipe) == -1){
			perror("Pipe Fails\n");
		    }
		    prevPipe[0] = curPipe[0];
		    prevPipe[1] = curPipe[1];
		}
	    }
	}
	close(prevPipe[0]);
	close(prevPipe[1]);
	close(curPipe[0]);
	close(curPipe[1]);
	
	dup2(keyboard, STDIN_FILENO);
	dup2(terminal, STDOUT_FILENO);
	if(argc == 1){
	    printf("8-D ");
	}
	if(argc == 1){
	    (fgets(orig, INPUTLIMIT + 2, stdin));
	}else{
	    fileRead(orig, filePtr);
	}
    }
    printf("\n");
    return 1;
}

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
