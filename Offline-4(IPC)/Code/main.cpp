#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<math.h>
#include"rand.c"
#include<algorithm>
#include<semaphore.h>
#include<time.h>


using namespace std;

/*    Function prototypes   */
void access_printing_machine(struct students *tmpStudent);
void leave_printing_machine(struct students *tmpStudent);
void test(struct students *tmpStudent);
int wakeup_groupmates(struct students *tmpStudent);
void wakeup_others(struct students *tmpStudent);

enum studentState {IDLE, WAITING, PRINTING};

struct students
{
    int ID;
    int group;
    int printing_station;
    int teamLeader;
    int arrival;
    enum studentState state;
};


int studentCount = 20;
int groupCount = 4;
int w;
int x;
int y;
int printerStatus[4];                               // 0 means available, 1 means busy
struct students studentsArr[500];
enum studentState studentStateArr[500];
pthread_mutex_t mutex_printing;                     // To enter critical region while printing
sem_t sem_student[500];                             // Semaphore for each student



void access_printing_machine(struct students *tmpStudent){
    pthread_mutex_lock(&mutex_printing);
    tmpStudent->state = WAITING;
    test(tmpStudent);
    pthread_mutex_unlock(&mutex_printing);
    sem_wait(&sem_student[tmpStudent->ID - 1]);

}

void leave_printing_machine(struct students *tmpStudent){
    pthread_mutex_lock(&mutex_printing);
    tmpStudent->state = IDLE;                               //Done printing, IDLE now
    printerStatus[tmpStudent->printing_station - 1] = 0;    //That printer is free now
    printf("Student %d has finished printing at time \n",tmpStudent->ID);
    int flag = wakeup_groupmates(tmpStudent);
    if(flag == 0) wakeup_others(tmpStudent);   //flag 0 means no group member woke up, so call others
    pthread_mutex_unlock(&mutex_printing);
}

void test(struct students *tmpStudent){
    if(printerStatus[tmpStudent->printing_station - 1] == 0){
        tmpStudent->state = PRINTING;
        printerStatus[tmpStudent->printing_station - 1] = 1;    //That printer is busy now
        printf("Student %d has arrived at the printing station at time \n",tmpStudent->ID);
        sem_post(&sem_student[tmpStudent->ID - 1]);
    }
}

int wakeup_groupmates(struct students *tmpStudent){
    int flag = 0;
    for(int i=0; i<studentCount; i++){
        //If that student is in the same group,assigned to same printing machine & waiting on that machine
        if(studentsArr[i].group == tmpStudent->group && studentsArr[i].printing_station == tmpStudent->printing_station && studentsArr[i].state == WAITING){
            studentsArr[i].state = PRINTING;
            printf("Student %d has arrived at the printing station at time \n",studentsArr[i].ID);
            sem_post(&sem_student[studentsArr[i].ID - 1]);
            flag = 1;
            if(flag == 1)break;
        }
    }
    return flag;
}


void wakeup_others(struct students *tmpStudent){
    int flag = 0;
    for(int i=0; i<studentCount; i++){
        //If that student is assigned to same printing machine & waiting on that machine
        if(studentsArr[i].printing_station == tmpStudent->printing_station && studentsArr[i].state == WAITING){
            studentsArr[i].state = PRINTING;
            printf("Student %d has arrived at the printing station at time \n",studentsArr[i].ID);
            sem_post(&sem_student[studentsArr[i].ID - 1]);
            flag = 1;
            if(flag == 1)break;
        }
    }
}

void * studentTask(void * arg){
    struct students *tmpStudent = (students*)arg;

    //Random delays before a student steps into the printing phase
    sleep(tmpStudent->arrival);
    //printf("ID: %d, group: %d, printing station: %d, team leader: %d arrival: %d\n",tmpStudent->ID, tmpStudent->group,tmpStudent->printing_station,tmpStudent->teamLeader,tmpStudent->arrival);
    access_printing_machine(tmpStudent);
    sleep(w);   // Print
    leave_printing_machine(tmpStudent);
}




int main(){

    

    studentCount = 20;
    groupCount = 4;
    w = 10;
    x = 8;
    y = 3;


    /*         Initialization Begins         */
    init_genrand(1905101);
    pthread_mutex_init(&mutex_printing,NULL);
    for(int i=0; i<studentCount; i++){
        sem_init(&sem_student[i],0,0);
    }

    //Initialize printing station status
    for(int i=0; i<4; i++){
        printerStatus[i] = 0;
    }

    //Initialize student's info
    for(int i=0; i<studentCount; i++){
        studentsArr[i].ID = i+1;
        studentsArr[i].group = floor(i/(studentCount/groupCount))+1;
        studentsArr[i].printing_station = ((i+1)%4)+1;
        if((i+1)%(studentCount/groupCount)== 0) studentsArr[i].teamLeader = 1;
        else studentsArr[i].teamLeader = 0;
        studentsArr[i].arrival = genrand_int31(max((int)20/2,1));
        studentsArr[i].state = IDLE;
    }
    for(int i=0; i<studentCount; i++){
        studentStateArr[i] = IDLE;
    }

    /*         Initialization ends                */

    //Initialize student threads
    pthread_t th[studentCount];
    for(int i=0; i<studentCount; i++){
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        pthread_create(&th[i], NULL, studentTask, (void*)&studentsArr[i]);
    }
    for(int i=0; i<studentCount;i++){
        pthread_join(th[i],NULL);
    }
    return 0;
}