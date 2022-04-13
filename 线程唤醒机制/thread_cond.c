#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

pthread_t atter;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t function1_cond = PTHREAD_COND_INITIALIZER;

int function_run = 0;

void *function1()
{
    pthread_mutex_lock(&mutex);
    printf("======= this is function  1 =====\n");
    while (1)
    {
        printf("======= this is function  2 =====\n");
        while (!function_run)
        {
            printf("======= this is function  3 ===\n");
            pthread_cond_wait(&function1_cond, &mutex);
        }
        printf("======= this is function  4=====\n");
        function_run = 0;
        pthread_mutex_unlock(&mutex);
    }
}

int create_pthread()
{
    if (-1 == pthread_create(&atter, NULL, function1, NULL))
    {
        printf("Create pthread failed");
        return -1;
    }
    pthread_detach(atter);
    return 0;
}

int main()
{
    create_pthread();
    sleep(2);

    printf("\n第一次唤醒线程\n");
    pthread_mutex_lock(&mutex);
    function_run = 1;
    pthread_cond_signal(&function1_cond);
    pthread_mutex_unlock(&mutex);

    sleep(2);
    printf("\n第二次唤醒线程\n");
    pthread_mutex_lock(&mutex);
    function_run = 1;
    pthread_cond_signal(&function1_cond);
    pthread_mutex_unlock(&mutex);

    sleep(2);
    printf("\n第三次唤醒线程\n");
    pthread_mutex_lock(&mutex);
    function_run = 1;
    pthread_cond_signal(&function1_cond);
    pthread_mutex_unlock(&mutex);

    sleep(2);
    printf("\n第四次唤醒线程\n");
    pthread_mutex_lock(&mutex);
    function_run = 1;
    pthread_cond_signal(&function1_cond);
    pthread_mutex_unlock(&mutex);

    sleep(2);
    pthread_detach(atter);
    return 0;
}
