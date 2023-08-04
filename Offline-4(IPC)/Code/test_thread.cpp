#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

void * runn(void * arg){
    printf("Hello world\n");
    sleep(3);
    printf("%s\n",(char*)arg);
    printf("Bye Bye\n");
}

int main()
{
    pthread_t t1,t2;
    pthread_create(&t1, NULL, runn, (void*)"Hello i am 1", (void*)"Hello i am 3");
    pthread_create(&t2, NULL, runn, (void*)"Hello i am 2", (void*)"Hello i am 4");

    pthread_join(t1,NULL);
    pthread_join(t2,NULL);

    return 0;
}