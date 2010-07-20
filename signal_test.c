#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define NUM_THREADS 2

void thread_sig_mask(void){
    sigset_t set;

    sigfillset(&set);
    pthread_sigmask(SIG_SETMASK, &set, NULL);
}

void *thread_run(void *param){
        thread_sig_mask();
        int i = 0;
        for (i = 0; i < 100; ++i) {
            fprintf(stdout, "\nTID=%u count=%d", (int) pthread_self(), i );
            sleep(1);
        }
        pthread_exit(0);
}

void thread_create(void){

    int rc = 0, i;

    int sig;
    sigset_t set, oldSigSet;

    pthread_t tid[NUM_THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // sigemptyset(&set);
    // sigaddset(&set, SIGINT);
    // pthread_sigmask(SIG_BLOCK, &set, NULL);

    for (i = 0; i < NUM_THREADS; ++i) {
        if ( 0 != (rc = pthread_create(&tid[i], NULL, thread_run, NULL)) ) {
            perror("thread create");
            return;
        }
    }

    printf("All threads created.\n");

    while( 1 ) {
        rc = sigwait(&set, &sig);
        printf("Signal received rc=%d.\n", rc);

        for (i = 0; i < NUM_THREADS; ++i) {
            if (0 != (rc = pthread_kill(tid[i], SIGINT)) ) {
                perror("thread kill");
                return;
            }
        }
        printf("Signal processed.\n");
    }


    for (i = 0; i < NUM_THREADS; ++i) {
        if (0 != (rc = pthread_join(tid[i], NULL)) ) {
            perror("thread join");
            return;
        }
    }
    printf("All threads exited.\n");

}

int main(int argc, char **argv){
    thread_create();
    return 0;
}
