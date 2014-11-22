#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "relax.h"

/*
 * Synchronisation control
 *
 * Do you want to hear a joke?
 * Apple's implementation of PThreads doesn't include Barriers!
 */
pthread_mutex_t mtx_idle = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cnd_idle = PTHREAD_COND_INITIALIZER;
int idle;

pthread_mutex_t mtx_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cnd_ready = PTHREAD_COND_INITIALIZER;
int ready;

pthread_mutex_t mtx_working = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cnd_working = PTHREAD_COND_INITIALIZER;
int working;

pthread_mutex_t mtx_finish = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cnd_finish = PTHREAD_COND_INITIALIZER;
int finish;

// threads increment this value if values in their rows
// are not yet within the required precision
// when this value remains 0, relaxation is complete
pthread_mutex_t mtx_not_complete = PTHREAD_MUTEX_INITIALIZER;
int not_complete;

/*
 * Destroy the mutexes and condition variables once
 * they are nolonger needed
 */
void destroy()
{
        pthread_mutex_destroy(&mtx_idle);
        pthread_mutex_destroy(&mtx_ready);
        pthread_mutex_destroy(&mtx_working);
        pthread_mutex_destroy(&mtx_finish);
        pthread_mutex_destroy(&mtx_not_complete);
        pthread_cond_destroy(&cnd_idle);
        pthread_cond_destroy(&cnd_ready);
        pthread_cond_destroy(&cnd_working);
        pthread_cond_destroy(&cnd_finish);
}

/*
 * Relax. Calculate the averages for rows in a given range
 */
void  * relax(void *ptr)
{
        struct work *w = (struct work *) ptr;
        int i, j;
        double sum, avg, diff;
        int flag = 1;

        while (1) {

                // set self to idle
                pthread_mutex_lock(&mtx_idle);
                idle++;
                pthread_cond_signal(&cnd_idle);
                pthread_mutex_unlock(&mtx_idle);

                // wait until told to start
                pthread_mutex_lock(&mtx_ready);
                while (!ready)
                        pthread_cond_wait(&cnd_ready, &mtx_ready);
                pthread_mutex_unlock(&mtx_ready);

                // first loop, with precision checks
                for (i = w->r->start; i < w->r->end; i++) {
                        for (j = 1; j < w->mats->size - 1; j++) {
                                // calculate the average of surrounding vals
                                sum = w->mats->imat[i - 1][j]
                                        + w->mats->imat[i][j + 1]
                                        + w->mats->imat[i + 1][j]
                                        + w->mats->imat[i][j - 1];
                                avg = sum / 4;
                                // store the result
                                w->mats->rmat[i][j] = avg;
                                diff = fabs(w->mats->imat[i][j] -
                                                w->mats->rmat[i][j]);
                                // is difference within tolerance?
                                if (diff > w->tolerance) {
                                        flag = 0;
                                        break;
                                }
                        }
                        if (!flag)
                                break;
                }
                // second loop, without precision checks
                for (; i < w->r->end; i++) {
                        for (; j < w->mats->size - 1; j++) {
                                sum = w->mats->imat[i - 1][j]
                                        + w->mats->imat[i][j + 1]
                                        + w->mats->imat[i + 1][j]
                                        + w->mats->imat[i][j - 1];
                                avg = sum / 4;
                                w->mats->rmat[i][j] = avg;
                        }
                        j = 1;
                }

                // if there are values not within the precision
                // increment the count
                if (!flag) {
                        pthread_mutex_lock(&mtx_not_complete);
                        not_complete++;
                        pthread_mutex_unlock(&mtx_not_complete);
                }
                // reset flag
                flag = 1;

                // set self to finished
                pthread_mutex_lock(&mtx_working);
                working--;
                pthread_cond_signal(&cnd_working);
                pthread_mutex_unlock(&mtx_working);

                // wait to finish
                pthread_mutex_lock(&mtx_finish);
                while (!finish)
                        pthread_cond_wait(&cnd_finish, &mtx_finish);
                pthread_mutex_unlock(&mtx_finish);
        }
        pthread_exit(NULL);
}

int check()
{
        int r;
        pthread_mutex_lock(&mtx_not_complete);
        if (not_complete > 0) {
                r = 1;
        } else {
                r = 0;
        }
        not_complete = 0;
        pthread_mutex_unlock(&mtx_not_complete);
        return r;
}

void swap(struct matrices *mats)
{
        double **tmp;
        tmp = mats->imat;
        mats->imat = mats->rmat;
        mats->rmat = tmp;
}

int main(int argc, char **argv)
{
        int size, numthr, prec, lenarr;
        int *arr;
        int i;

        // CL Argument handling
        if (argc < 4) {
                fprintf(stderr, "Error: Too few arguments\n");
                exit(1);
        } else {
                size = atoi(argv[1]);
                numthr = atoi(argv[2]);
                prec = atoi(argv[3]);
                lenarr = size * size;
        }
        if (argc == 4) {
                arr = createrandom(lenarr);
                /*arr = createl(size);*/
        } else {
                arr = createarr(argc, argv, lenarr);
        }

        // Initialise structs with values
        struct matrices *mats = malloc(sizeof(struct matrices));
        initmats(mats, arr, size);

        // Partition matrix so each thread works equally
        struct range *ranges = partmat(size, numthr);

        // wrap up mats, range and precision into single struct
        struct work *w = malloc(numthr * sizeof(struct work));
        // reserve memory for each thread
        pthread_t *thr = malloc(numthr * sizeof(pthread_t));

        // create work for each thread & start thread
        for (i = 0; i < numthr; i++) {
                w[i].mats = mats;
                w[i].r = &ranges[i];
                w[i].tolerance = (double) 1 / prec;
                pthread_create(&thr[i], NULL, (void *) &relax, (void *) &w[i]);
        }

        // main loop
        int numits = 0;
        do {
                // wait for all threads to become ready
                pthread_mutex_lock(&mtx_idle);
                while (idle != numthr)
                        pthread_cond_wait(&cnd_idle, &mtx_idle);
                pthread_mutex_unlock(&mtx_idle);

                // stop threads from finishing prematurely
                finish = 0;
                // all threads will now be working
                working = numthr;

                // signal threads to start
                pthread_mutex_lock(&mtx_ready);
                ready = 1;
                pthread_cond_broadcast(&cnd_ready);
                pthread_mutex_unlock(&mtx_ready);

                // wait for threads to finish
                pthread_mutex_lock(&mtx_working);
                while (working != 0)
                        pthread_cond_wait(&cnd_working, &mtx_working);
                pthread_mutex_unlock(&mtx_working);

                // swap matrices
                swap(mats);

                // prevent threads from restarting
                ready = 0;
                idle = 0;

                // signal threads to finish
                pthread_mutex_lock(&mtx_finish);
                finish = 1;
                pthread_cond_broadcast(&cnd_finish);
                pthread_mutex_unlock(&mtx_finish);

                numits++;
        } while (check());

        // deallocate memory
        freemat(mats->imat, mats->size);
        freemat(mats->rmat, mats->size);
        free(mats);
        free(ranges);
        free(w);
        free(thr);
        destroy();

        printf("%d\n", numits);
        return 0;
}
