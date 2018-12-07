#ifndef STAGE
#define STAGE

struct stage{
    int stageNum;
    char *input; //If input is NULL it is a pipe
    char *output; //If output is NULL it is in a pipe
    int argumentCount;
    char **argumentVar;
} stage;

int scanline(char *line);
struct stage *pipePrint(int pipeNum, char *input, char *output,
                char *pipeLine);
struct stage **correctString(char *string);
char *copyToSpace(char *source, char *copy);
struct stage *createStage(int stageN);
struct stage **parseline(char *orig);

#endif
