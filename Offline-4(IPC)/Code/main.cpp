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
void doBinding(struct students *tmpStudent);
void write_entry_book(struct students *tmpStudent);
void read_entry_book(int id);

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


int studentCount;
int groupCount;
int w;
int x;
int y;
int printerStatus[4];                               // 0 means available, 1 means busy
                                  
vector<students> studentsArr;
pthread_mutex_t mutex_printing;                     // To enter critical region while printing
vector<sem_t> sem_student;                          // Semaphore for each student
time_t starttime = time(NULL);
vector<int> printCount;
sem_t sem_binding;                                  // Seamphore for binding...Initialized to 2
sem_t sem_submission;                               // To control reader,writer's access to submission count
pthread_mutex_t mutex_submission;
int reader_count = 0;
int submission_count = 0;



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
    printf("Student %d has finished printing at time %ld\n",tmpStudent->ID,(time(NULL)-starttime));

    //Update printcount and wakeup leader if he is sleeping
    printCount[tmpStudent->group - 1]++;
    if(printCount[tmpStudent->group - 1] == (studentCount/groupCount)){
        sem_post(&sem_student[(tmpStudent->group)*(studentCount/groupCount) - 1]);
    }

    int flag = wakeup_groupmates(tmpStudent);
    if(flag == 0) wakeup_others(tmpStudent);   //flag 0 means no group member woke up, so call others
    pthread_mutex_unlock(&mutex_printing);
}

void test(struct students *tmpStudent){
    if(printerStatus[tmpStudent->printing_station - 1] == 0){
        tmpStudent->state = PRINTING;
        printerStatus[tmpStudent->printing_station - 1] = 1;    //That printer is busy now
        printf("Student %d has arrived at the printing station at time %ld\n",tmpStudent->ID, (time(NULL)-starttime));
        sem_post(&sem_student[tmpStudent->ID - 1]);
    }
}

int wakeup_groupmates(struct students *tmpStudent){
    int flag = 0;
    for(int i=0; i<studentCount; i++){
        //If that student is in the same group,assigned to same printing machine & waiting on that machine
        if(studentsArr[i].group == tmpStudent->group && studentsArr[i].printing_station == tmpStudent->printing_station && studentsArr[i].state == WAITING){
            studentsArr[i].state = PRINTING;
            //printf("****Student %d has woke up his groupmate %d ****\n",tmpStudent->ID,studentsArr[i].ID);
            printf("Student %d has arrived at the printing station at time %ld\n",studentsArr[i].ID,(time(NULL)-starttime));
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
            //printf("-----Student %d has woke up other person %d ------\n",tmpStudent->ID,studentsArr[i].ID);
            printf("Student %d has arrived at the printing station at time %ld\n",studentsArr[i].ID, (time(NULL)-starttime));
            sem_post(&sem_student[studentsArr[i].ID - 1]);
            flag = 1;
            if(flag == 1)break;
        }
    }
}


void doBinding(struct students *tmpStudent){

    sem_wait(&sem_binding);
    printf("Group %d has started binding at time %ld\n", tmpStudent->group, (time(NULL)-starttime));
    sleep(x);
    printf("Group %d has finished binding at time %ld\n", tmpStudent->group, (time(NULL)-starttime));
    sem_post(&sem_binding);
}

void write_entry_book(struct students *tmpStudent){
    sem_wait(&sem_submission);
    sleep(y);
    submission_count++;
    printf("Group %d has submitted the report at time %ld\n",tmpStudent->ID,(time(NULL)-starttime));
    sem_post(&sem_submission);
}

void read_entry_book(int id){
    while(true){
        pthread_mutex_lock(&mutex_submission);
        reader_count++;
        if(reader_count == 1) sem_wait(&sem_submission);        //The first reader
        pthread_mutex_unlock(&mutex_submission);

        // Read the data
        sleep(y);
        printf("Staff %d has started reading the entry book at time %ld. No. of submission = %d\n",id, (time(NULL)-starttime),submission_count);
        if(submission_count == groupCount){
            // All submission done
            break;
        }

        pthread_mutex_lock(&mutex_submission);
        reader_count--;
        if(reader_count == 0) sem_post(&sem_submission);        // The last reader
        pthread_mutex_unlock(&mutex_submission);

        // Now sleep...come back later
        int rand = genrand_int31(10);
        //printf("-------------  Staff %d , rand: %d\n",id,rand);
        sleep(rand);
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
    if(tmpStudent->teamLeader){
        sem_wait(&sem_student[tmpStudent->ID - 1]);
    }
    else{
        pthread_exit(NULL);
    }

    // Only group leader can access the below section
    // pthread_exit will ensure that
    // Print finish printing messages
    printf("Group %d has finished printing at time %ld\n",tmpStudent->group, (time(NULL)-starttime)); 

    // Do binding 
    doBinding(tmpStudent);

    // Write entry-book
    write_entry_book(tmpStudent);
}


void * staffTask(void * arg){
    int* id = (int*)arg;
    int rand = genrand_int31(5);
    sleep(rand);
    read_entry_book(*id);
}




int main(){
    studentCount = 20;
    groupCount = 4;
    w = 10;
    x = 8;
    y = 3;


    /*         Initialization Begins         */

    //Resize
    sem_student.resize(studentCount + 1);
    studentsArr.resize(studentCount + 1);

    //Mutex and semaphore initialization
    init_genrand(1905101);
    pthread_mutex_init(&mutex_printing,NULL);
    pthread_mutex_init(&mutex_submission, NULL);
    sem_init(&sem_binding,0,2);
    sem_init(&sem_submission,0,1);
    for(int i=0; i<studentCount; i++){
        sem_init(&sem_student[i],0,0);
    }

    //Initialize printing station status
    for(int i=0; i<4; i++){
        printerStatus[i] = 0;
    }

    //Initialize print count status
    printCount.resize(groupCount + 1);
    for(int i=0; i<groupCount; i++){
        printCount[i] = 0;
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


    /*         Initialization ends                */

    //Initialize student threads
    pthread_t th[studentCount];
    for(int i=0; i<studentCount; i++){
        pthread_create(&th[i], NULL, studentTask, (void*)&studentsArr[i]);
    }

    //Initialize staff threads
    pthread_t staff1, staff2;
    int* id1 = (int*)malloc(sizeof(int));
    *id1 = 1;
    pthread_create(&staff1, NULL, staffTask, (void*)id1);
    int* id2 = (int*)malloc(sizeof(int));
    *id2 = 2;
    pthread_create(&staff2, NULL, staffTask, (void*)id2);


    // Join the threads
    for(int i=0; i<studentCount;i++){
        pthread_join(th[i],NULL);
    }
    pthread_join(staff1, NULL);
    pthread_join(staff2, NULL);

    //Distroy semaphore and mutex
    for(int i=0; i<studentCount; i++){
        sem_destroy(&sem_student[i]);
    }
    pthread_mutex_destroy(&mutex_printing);
    pthread_mutex_destroy(&mutex_submission);
    sem_destroy(&sem_binding);
    sem_destroy(&sem_submission);
    return 0;
}