//#ifndef YASH_JOBLISTS_H
//#define YASH_JOBLISTS_H
//
//#define running 0
//#define stopped 1
//#define done 2
//#define fg 3
//
//#include <sys/types.h>
//
//typedef struct Job Job;
//
//struct Job{
//    char** cmd;
//    int status;
//    pid_t pgid;
//    int position;
//
//    Job *next;
//};
//Job *head;
//Job *mostRecentJob;
//
//int delete(pid_t pgid);
//void add(Job *newJob);
//Job* find(pid_t pgid);
//void cleanLinkedList();
//void setMostRecentJob();
//
//#endif //YASH_JOBLISTS_H
#ifndef YASH_JOBLISTS_H
#define YASH_JOBLISTS_H

#define running 0
#define stopped 1
#define done 2
#define fg 3

#include <sys/types.h>

typedef struct Job Job;

struct Job{
    char** cmd;
    int status;
    pid_t pgid;
    int position;

    Job *next;
};
Job *head;
Job *foregroundJob;
int delete(pid_t pgid);
void add(Job *newJob);
Job* find(pid_t pgid);
void cleanLinkedList();
Job* getMostRecentJob();

#endif //YASH_JOBLISTS_H

