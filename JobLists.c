//#include <string.h>
//#include <stdlib.h>
//#include "JobLists.h"
//void deleteJob(Job *currentJob){
//    int i = 0;
//    while((currentJob->cmd)[i] != 0){
//        free((currentJob->cmd)[i]);
//        i++;
//    }
//    free((currentJob->cmd));
//    free(currentJob);
//}
//int delete(pid_t pgid){
//    if(head == NULL){
//        return -1;
//    }
//    Job *prev = NULL;
//    Job *temp = head;
//    while(temp != NULL){
//        if(temp->pgid == pgid){
//            break;
//        }
//        prev = temp;
//        temp = temp->next;
//    }
//    if(temp == NULL){
//        return -1;
//    }
//    if(prev == NULL){
//        if(temp->next == NULL){
//            head = NULL;
//        }else{
//            head = temp->next;
//        }
//    }else{
//        prev->next = temp->next;
//    }
//    deleteJob(temp);
//    return 0;
//}
//void add(Job *newJob){
//    if(head == NULL){
//        head = newJob;
//        head->position = 1;
//        return;
//    }
//    Job *temp = head;
//    while(temp->next != NULL){
//        temp = temp->next;
//    }
//    temp->next = newJob;
//    newJob->position = temp->position + 1;
//}
//Job* find(pid_t pgid){
//    if(head == NULL){
//        return NULL;
//    }
//    Job *temp = head;
//    while(temp->pgid != pgid && temp->next != NULL){
//        temp = temp->next;
//    }
//    if(temp == NULL){
//        return NULL;
//    }else{
//        return temp;
//    }
//}
//void cleanLinkedList(){
//    if(head == NULL){
//        return;
//    }
//    Job *current;
//    Job *next = head;
//    do{
//        current = next;
//        next = current->next;
//        deleteJob(current);
//    }while(next != NULL);
//}
//
//void setMostRecentJob(){
//    if(head == NULL){
//        mostRecentJob = NULL;
//        return;
//    }
//    Job* current = head;
//    while(current->next != NULL){
//        current = current->next;
//    }
//    mostRecentJob = current;
//}

#include <string.h>
#include <stdlib.h>
#include "JobLists.h"
void deleteJob(Job *currentJob){
    int i = 0;
    while((currentJob->cmd)[i] != 0){
        free((currentJob->cmd)[i]);
        i++;
    }
    free((currentJob->cmd));
    free(currentJob);
}
int delete(pid_t pgid){
    if(head == NULL){
        return -1;
    }
    Job *prev = NULL;
    Job *temp = head;
    while(temp != NULL){
        if(temp->pgid == pgid){
            break;
        }
        prev = temp;
        temp = temp->next;
    }
    if(temp == NULL){
        return -1;
    }
    if(prev == NULL){
        if(temp->next == NULL){
            head = NULL;
        }else{
            head = temp->next;
        }
    }else{
        prev->next = temp->next;
    }
    deleteJob(temp);
    return 0;
}
void add(Job *newJob){
    if(head == NULL){
        head = newJob;
        head->position = 1;
        return;
    }
    Job *temp = head;
    while(temp->next != NULL){
        temp = temp->next;
    }
    temp->next = newJob;
    newJob->position = temp->position + 1;
}
Job* find(pid_t pgid){
    if(head == NULL){
        return NULL;
    }
    Job *temp = head;
    while(temp->pgid != pgid && temp->next != NULL){
        temp = temp->next;
    }
    if(temp == NULL){
        return NULL;
    }else{
        return temp;
    }
}
void cleanLinkedList(){
    if(head == NULL){
        return;
    }
    Job *current;
    Job *next = head;
    do{
        current = next;
        next = current->next;
        deleteJob(current);
    }while(next != NULL);
}
Job* getMostRecentJob(){
    if(head == NULL){
        return NULL;
    }
    Job* current = head;
    while(current->next != NULL){
        current = current->next;
    }
    return current;
}
