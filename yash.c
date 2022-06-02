#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "JobLists.h"

#define charsPerToken 30
#define maxCharPerLine 2000
#define presetPermissions (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)
#define TRUE 1
#define FALSE 0

//stdin == 0;
//stdout == 1;
//stderr == 2;

int flag = FALSE;

void sighandler(int signo){
    if(signo == SIGCHLD){
        int status;
        pid_t cpid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
        if(cpid == -1){
            return;
        }
        if(foregroundJob != NULL){
            if(foregroundJob->pgid == cpid){
                flag = TRUE;
            }
        }
        if(WIFEXITED(status)){  //normal exit
            Job *currentJob = find(cpid);
            if(currentJob == NULL) {    //should never happen, somehow a job got past bookkeeping
                return;
            }
            if (currentJob->status == fg) {
                flag = TRUE;
                delete(currentJob->pgid);
            }else{
                currentJob->status = done;
            }
        }else if(WIFSTOPPED(status)){
            Job *currentJob = find(cpid);
            if(currentJob != NULL){
                currentJob->status = stopped;
            }
        }else if(WIFCONTINUED(status)){
            Job *currentJob = find(cpid);
            if(currentJob != NULL){
                currentJob->status = running;
            }
        }
    }else if(signo == SIGINT){
        if(foregroundJob != NULL){
            killpg((foregroundJob->pgid), SIGINT);
            delete(foregroundJob->pgid);
        }
    }else if(signo == SIGTSTP){
        if(foregroundJob != NULL){
            killpg((foregroundJob->pgid), SIGTSTP);
            foregroundJob->status = stopped;
        }
    }
}
void freeWordList(char **wordList){
    int i = 0;
    while(wordList[i] != 0){
        free(wordList[i]);
        i++;
    }
    free(wordList);
}
char **parseString(char *inString, int *pipePresent, int *bg){
    int i = 0;
    int count = 0;
    //char** wordList = calloc(0,(maxCharPerLine/charsPerToken) * sizeof(char*));
    char** wordList = calloc((maxCharPerLine/charsPerToken), sizeof(char));
    while(inString[i] != 0){
        while(inString[i] == ' '){
            i++;    //skip spaces
        }
        int j = 0;
        //char* nextWord = calloc(0,charsPerToken * sizeof(char));
        char* nextWord = calloc(charsPerToken,sizeof(char));
        while(inString[i] != ' ' && inString[i] != 0){
            nextWord[j] = inString[i];
            i++;
            j++;
        }

        if(strcmp(nextWord, "|") == 0){
            *pipePresent = TRUE;
        }

        wordList[count] = nextWord;
        count++;

        while(inString[i] == ' '){  //just a precaution if the last "any" characters are spaces
            i++;    //skip spaces
        }
    }

    if(count > 0 && strcmp(wordList[count - 1], "&") == 0){  //make note if job is background and clean up the last token
        *bg = TRUE;
        count--;
        free(wordList[count]);
        wordList[count] = NULL;
    }

    //*wordCount = count;
    return wordList;
}
void executeShellCmdJobs(void){
    Job* current = head;
    while(current != NULL){
        char sign = '-';
        if(current->next == NULL){  //most recent
            sign = '+';
        }
        char status[10];

        if(current->status == running){
            strcpy(status,"Running");
        }else if(current->status == stopped){
            strcpy(status,"Stopped");
        }else{
            strcpy(status,"Done   ");
        }
        printf("[%d]%c %s       ",current->position,sign,status);
        int j = 0;
        while(current->cmd[j] != NULL){
            printf(" %s", current->cmd[j]);
            j++;
        }
        if(current->status == running){
            printf(" &");
        }
        printf("\n");
        if(current->status == done){
            Job* temp = current;
            current = current->next;
            delete(temp->pgid);
        }else{
            current = current->next;
        }
    }
}
void executeShellCmdFG(void){
    Job* mostRecentJob;
    if((mostRecentJob = getMostRecentJob()) == NULL){
        return;
    }
    int i = 0;
    while(mostRecentJob->cmd[i] != 0){
        printf("%s ",mostRecentJob->cmd[i]);
        i++;
    }
    printf("\n");
    int prevStatus = mostRecentJob->status;
    mostRecentJob->status = fg;
    killpg((mostRecentJob->pgid),SIGCONT);
    if(prevStatus != running){
        pause(); //catch and ignore sigcont handle
    }
    flag = FALSE;
    foregroundJob = mostRecentJob;
    do{
        pause();
    }while(!flag);
    flag = FALSE;
    foregroundJob = NULL;
}
void cleanRunningPrograms(){
    Job* current = head;
    while(current != NULL){
        killpg(current->pgid, SIGINT);
        current = current->next;
    }
}
void executeShellCmdBG(void){
    Job* stoppedJob = NULL;
    Job* current = head;
    while(current != NULL){
        if(current->status == stopped){
            stoppedJob = current;
        }
        current = current->next;
    }
    if(stoppedJob == NULL) {
        return;
    }
    char status[8] = "Running";

    printf("[%d]+ %s       ",stoppedJob->position,status);
    int j = 0;
    while(stoppedJob->cmd[j] != NULL){
        printf(" %s", stoppedJob->cmd[j]);
        j++;
    }
    printf(" &\n");

    stoppedJob->status = running;
    killpg(stoppedJob->pgid,SIGCONT);
}
void executeShellCmd(char **parsedCmd){
    if(parsedCmd[1] != NULL){
        printf("invalid shell cmd");
        return;
    }
    if(strcmp(parsedCmd[0], "jobs") == 0){
        executeShellCmdJobs();
    }else if(strcmp(parsedCmd[0], "fg") == 0){
        executeShellCmdFG();
    }else if(strcmp(parsedCmd[0], "bg") == 0){
        executeShellCmdBG();
    }else{
        printf("error: something went wrong while parsing shell cmd");
    }
    freeWordList(parsedCmd);
}
void executeProcess(char** parsedCmd, int pipeExit, char** cmd){
    int i = 0;
    int offset = 0; //only useful when the second process of a pipe uses this method
    if(pipeExit == TRUE){       //if this process is the "exit" of the pipe, then proceeds after the start of the second process is found
        while(strcmp(parsedCmd[offset], "|") != 0){
            offset++;
        }
        offset++;
    }
    while(parsedCmd[i + offset] != 0){
        if((strcmp(parsedCmd[i + offset], ">") == 0) || (strcmp(parsedCmd[i + offset], "<") == 0)
           || (strcmp(parsedCmd[i + offset],"2>") == 0) || (strcmp(parsedCmd[i + offset], "|") == 0)){
            break;
        }
        cmd[i] = parsedCmd[i + offset];  //sharing the pointer to the "main" cmd
        i++;
    }
    cmd[i] = NULL;
    while(parsedCmd[i + offset] != 0){   //now we are going to take care of the all the redirection operations
        if(strcmp(parsedCmd[i + offset], "|") == 0){
            break;                          //if pipe then exit because first part of process is found/done
            //if no pipe, then condition never reaches so its good
        }
        if((strcmp(parsedCmd[i + offset], ">") == 0)){    //output redirection
            i++;
            int fd = open(parsedCmd[i + offset], O_CREAT | O_WRONLY,presetPermissions);
            dup2(fd, 1);
        }else if((strcmp(parsedCmd[i + offset], "<") == 0)){  //input redirection
            i++;
            int fd = open(parsedCmd[i + offset], O_RDONLY, presetPermissions);
            if(fd < 0){
                printf("file %s does not exist\n", parsedCmd[i + offset]);
                exit(-1);
            }
            dup2(fd, 0);
        }else if((strcmp(parsedCmd[i + offset], "2>") == 0)){ //error redirection
            i++;
            int fd = open(parsedCmd[i + offset], O_CREAT | O_WRONLY, presetPermissions);
            dup2(fd, 2);
        }
        i++;
    }
}
pid_t executeJob(char** parsedCmd, int pipePresent){
    if(pipePresent == TRUE){
        char* cmd1[maxCharPerLine/charsPerToken];
        char* cmd2[maxCharPerLine/charsPerToken];
        int pfd[2];
        pipe(pfd);
        pid_t ch1,ch2;

        ch1 = fork();
        if(ch1 < 0){
            printf("fork was unsuccessful");
            return -1;
        }
        if(ch1 == 0){
            setpgid(0,0);
            signal(SIGTTOU,SIG_IGN);
            signal(SIGINT,SIG_DFL);
            signal(SIGTSTP,SIG_DFL);

            close(pfd[0]);
            dup2(pfd[1],1);
            executeProcess(parsedCmd, FALSE, cmd1);

            
            if(execvp(cmd1[0], cmd1)){
                exit(-1);          
            }
        }
        ch2 = fork();
        if(ch2 < 0){
            printf("fork was unsuccessful");
            return -1;
        }
        if(ch2 == 0){
            setpgid(0,ch1);
            signal(SIGTTOU,SIG_IGN);
            signal(SIGINT,SIG_DFL);
            signal(SIGTSTP,SIG_DFL);

            close(pfd[1]);
            dup2(pfd[0],0);
            executeProcess(parsedCmd, TRUE,cmd2);

            if(-1 == execvp(cmd2[0], cmd2)){
                exit(-1);           
            }
        }
        close(pfd[0]);
        close(pfd[1]);
        return ch1;
    }else{
        pid_t cpid = fork();
        if(cpid < 0){
            printf("fork was unsuccessful");
            return -1;
        }
        char* cmd[maxCharPerLine/charsPerToken];
        if(cpid == 0){
            setpgid(0,0);
            signal(SIGTTOU,SIG_IGN);
            signal(SIGINT,SIG_DFL);
            signal(SIGTSTP,SIG_DFL);

            executeProcess(parsedCmd, FALSE, cmd);
            if(-1 == execvp(cmd[0], cmd)){
                exit(-1);           
            }
        }
        return cpid;
    }
}

int main(void){
    char** parsedCmd;
    char* inString;
    if(signal(SIGTTOU,SIG_IGN) == SIG_ERR){ //shell ignores sigttou
        printf("Parent SIGTTOU ignore installation failed");
    }
    if(signal(SIGINT,sighandler) == SIG_ERR){
        printf("Parent SIGINT ignore installation failed");
    }
    if(signal(SIGTSTP,sighandler) == SIG_ERR){
        printf("Parent SIGTSTP ignore installation failed");
    }

    if(signal(SIGCHLD,sighandler) == SIG_ERR){
        printf("Parent SIGCHLD handler installation failed");
    }

    while(inString = readline("# ")){
        if(strcmp(inString,"") == 0){
            continue;
        }
        int pipePresent = FALSE;
        int bg = FALSE;
        int status;

        parsedCmd = parseString(inString, &pipePresent, &bg);
        if((strcmp(parsedCmd[0], "jobs") == 0) ||
           (strcmp(parsedCmd[0], "fg") == 0) ||
           (strcmp(parsedCmd[0], "bg") == 0)){
            executeShellCmd(parsedCmd);
            free(inString);
            continue;
        }
        pid_t pgid = executeJob(parsedCmd,pipePresent);
        if(pgid < 0){
            continue;
        }
        if(bg == FALSE){
            Job *newJob = (Job*)(malloc(1 * sizeof(Job)));
            newJob->status = fg;
            newJob->next = NULL;
            newJob->cmd = parsedCmd;
            newJob->pgid = pgid;
            add(newJob);
            foregroundJob = newJob;
            flag = FALSE;
            do{
                pause();
            }while(!flag);
            flag = FALSE;
            foregroundJob = NULL;
        }else{
            Job *newJob = (Job*)(malloc(1 * sizeof(Job)));
            newJob->status = running;
            newJob->next = NULL;
            newJob->cmd = parsedCmd;
            newJob->pgid = pgid;
            add(newJob);
        }
        free(inString);
    }
    cleanRunningPrograms();
    cleanLinkedList();
    return 0;
}
