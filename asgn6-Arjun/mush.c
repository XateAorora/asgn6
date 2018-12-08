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
int fillToSpace(char *output, int fd);


static void handlr(int signum)
{
    printf("\n");
    interrupted = 1;
}

int fillToSpace(char *output, int fd){
    int i = 0;
    int check = 1;
    for(; i < INPUTLIMIT + 2; i++){
	check = read(fd, &(output[i]), sizeof(char));
	if(output[i] == 10){
	    return 0;
	}
	if(check == 0){
	    return 1;
	}
    }
    perror("Command Line Too Long\n");
    exit(127);
    return 0;
}
    
int main(int argc, char *argv[]){
    struct sigaction sahint;
    sahint.sa_handler = handlr;
    sigaction(SIGINT, &sahint, NULL);


    
    
    sigset_t x;
    sigemptyset (&x);
    sigaddset(&x, SIGINT);

    
    int terminal = dup(1);
    int keyboard = dup(0);
    
    

    char orig[INPUTLIMIT + 2] = {'\0'};
    int fileDiscriptor = -1;
    
    
    if (argc >= 2){
	//read from file: if EOF, exit = 1;
	if((fileDiscriptor = open(argv[1], O_RDONLY)) == -1){
	    perror(argv[1]);
	    return 1;
	}
	exited = fillToSpace(orig, fileDiscriptor);
    }else{
	do{
	    orig[0] = 0;
	    interrupted = 0;
	    if (isatty(fileno(stdin)) || isatty(fileno(stdout))){
		printf("8-D ");
	    }
	    fgets(orig, INPUTLIMIT + 2, stdin);
	}while((exited == 0 && interrupted == 1) || orig[0] == 10);
	exited = feof(stdin);
	//printf("\n%i\n", orig[0]);
    }
    
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
	    }else{
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
			    //printf("reading back\n");
			    dup2(prevPipe[0], STDIN_FILENO);
			    close(prevPipe[0]);
			    close(prevPipe[1]);
			}else if(stages[i]->input != NULL){
			    input = open(stages[i]->input, O_RDONLY);
			    dup2(input, STDIN_FILENO);
			}
		    
			if(stages[i + 1] != NULL){
			    //printf("reading out\n");
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
			perror(stages[i]->argumentVar[0]);
			_exit(CHILDERROR);
		    }else{
			sigprocmask(SIG_UNBLOCK, &x, NULL);
			int *check = malloc(sizeof(int));
			*check = -2;
			close(prevPipe[0]);
			close(prevPipe[1]);
			waitpid(childAddress, check, WCONTINUED);
			if(*check == CHILDERROR){
			    perror("Child Process Failed!");
			    exit(CHILDERROR);
			}else{
			    //printf("Process Complete\n");
			}
			if(pipe(prevPipe) == -1){
			    perror("Pipe Fails\n");
			}
			prevPipe[0] = curPipe[0];
			prevPipe[1] = curPipe[1];
		    }
		}
		stages = NULL;
		close(prevPipe[0]);
		close(prevPipe[1]);
		close(curPipe[0]);
		close(curPipe[1]);
	    }
	}

	interrupted = 0;
	dup2(keyboard, STDIN_FILENO);
	dup2(terminal, STDOUT_FILENO);

	if (exited == 0){
	    if (argc >= 2){
		exited = fillToSpace(orig, fileDiscriptor);
	    }else{
		do{
		    orig[0] = 0;
		    interrupted = 0;
		    if (isatty(fileno(stdin)) || isatty(fileno(stdout))){
			printf("8-D ");
		    }
		    fgets(orig, INPUTLIMIT + 2, stdin);
		}while((exited == 0 && interrupted == 1) || orig[0] == 10);
		exited = feof(stdin);
		//printf("\n%i\n", orig[0]);
	    }
	}
		
    }
    if(argc == 1){
	    printf("\n");
    }
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
