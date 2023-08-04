#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
struct students
{
    /* data */
};


void * studentTask(void * arg){
    int roll = *(int*)arg;
    printf("Hello, I am %d\n",roll+1);
    free(arg);
}




int main(){
    int studentCount = 20;
    pthread_t th[studentCount];

    for(int i=0; i<studentCount; i++){
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        pthread_create(&th[i], NULL, studentTask, (void*)id);
    }
    for(int i=0; i<studentCount;i++){
        pthread_join(th[i],NULL);
    }
    return 0;
}