/*#include <getopt.h>*/
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*
 * Synchronisation control
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

/*
 * Structs
 */
struct range {
        int start;
        int end;
};

struct matrices {
        double **imat;
        double **rmat;
        int size;
};

// struct to wrap up a single range and pointers to the matrices
struct work {
        struct matrices *mats;
        struct range *r;
};

void printmat(struct matrices *mat)
{
        int i, j;
        for (i = 0; i < mat->size; i++) {
                for (j = 0; j < mat->size; j++) {
                        printf("%f\t", mat->imat[i][j]);
                }
                printf("\n");
        }
}

/*
 * Create a matrix of given size
 */
double** createmat(int size)
{
        int i;
        double **mat;
        mat = malloc(size * sizeof(double *));
        for (i = 0; i < size; i++) {
                mat[i] = malloc(size * sizeof(double));
                /*if (mat[i] == NULL) { fprintf(stderr, "out of memory\n"); }*/
        }
        return mat;
}

/*
 * Initialise a matrix with values
 */
void initmat(double **mat, int size, int *arr)
{
        int i, j;
        int tmp = 0;
        for (i = 0; i < size; i++) {
                for (j = 0; j < size; j++) {
                        mat[i][j] = arr[tmp];
                        tmp++;
                }
        }
}

/*
 * Create an arr of a given length
 */
int* createarr(int size)
{
        int i;
        int *arr = malloc(size * sizeof(int));
        srand(time(NULL));
        for (i = 0; i < size; i++) {
                arr[i] = rand() % 10;
        }
        return arr;
}

/*
 * freemat a matrix of given size
 */
void freemat(double **mat, int size)
{
        int i;
        for (i = 0; i < size; i++) {
                free(mat[i]);
        }
        free(mat);
}

/*
 * Find the inner size of a matrix
 * really simple, but saves on confusion
 */
int inrsize(int size)
{
        return size - 2;
}

/*
 * 'Partition' the matrix so each thread has roughly equal work
 */
struct range* partmat(int size, int numthr)
{
        int isize = inrsize(size);
        int unallocated = isize;
        int currthr = 0;
        int i;

        int *allocation = malloc(isize * sizeof(int));
        for (i = 0; i < isize; i++) {
                allocation[i] = 0;
        }

        while (unallocated > 0) {
                allocation[currthr]++;
                unallocated--;
                currthr++;
                if (currthr == numthr) currthr = 0;
        }

        // work out start/end points
        struct range *ranges = malloc(numthr * sizeof(struct range));
        for (i = 0; i < numthr; i++) {
                if (i == 0) {
                        ranges[i].start = 1;
                        ranges[i].end = ranges[i].start
                                + allocation[i];
                } else {
                        ranges[i].start = ranges[i - 1].start
                                + allocation[i - 1];
                        ranges[i].end = ranges[i].start
                                + allocation[i];
                }
        }
        free(allocation);
        return ranges;
}


/*
 * Relax. Calculate the averages for rows in a given range
 */
/*void relax(struct matrices *mats, struct range r)*/
void  * relax(void *ptr)
{
        struct work *w = (struct work *) ptr;
        int i, j;
        double sum, avg;

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

                /*sleep(1);*/
                // do work
                for (i = w->r->start; i < w->r->end; i++) {
                        for (j = 1; j < w->mats->size - 1; j++) {
                                sum = w->mats->imat[i - 1][j]
                                        + w->mats->imat[i][j + 1]
                                        + w->mats->imat[i + 1][j]
                                        + w->mats->imat[i][j - 1];
                                avg = sum / 4;
                                w->mats->rmat[i][j] = avg;
                        }
                }

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

int check(struct matrices *mats, double prec)
{
        int i, j;
        double diff;
        int matching = 0;
        double tolerance = 1 / prec;

        for (i = 0; i < mats->size; i++) {
                for (j = 0; j < mats->size; j++) {
                        diff = fabs(mats->imat[i][j] - mats->rmat[i][j]);
                        if (diff > tolerance) {
                                matching = 1;
                                return matching;
                        }
                }
        }
        return matching;
}

void swap(struct matrices *mats)
{
        double **tmp;
        tmp = mats->imat;
        mats->imat = mats->rmat;
        mats->rmat = tmp;
}


/*
 * Main
 * TODO:
 *  - Remove automatic generation of maitrces for final submission
 */
int main(int argc, char **argv)
{
        int size, numthr, prec, lenarr;
        int *arr;
        int i;

        /**************************************************/
        // CL Argument handling
        if (argc < 4) {
                fprintf(stderr, "Error: Too few arguments\n");
                exit(1);
        } else {
                size = atoi(argv[1]);
                numthr = atoi(argv[2]);
                prec = atoi(argv[3]);
                lenarr = size * size;
                arr = createarr(lenarr);
        }

        /**************************************************/
        // Initialise structs with values
        struct matrices *mats = malloc(sizeof(struct matrices));
        mats->imat = createmat(size);
        mats->rmat = createmat(size);
        mats->size = size;
        initmat(mats->imat, mats->size, arr);
        initmat(mats->rmat, mats->size, arr);
        free(arr);

        // Partition matrix so each thread works equally
        struct range *ranges = partmat(size, numthr);

        // wrap up mats and a range
        struct work *w = malloc(numthr * sizeof(struct work));
        pthread_t *thr = malloc(numthr * sizeof(pthread_t));
        /**************************************************/
        // Work item for each thread & start thread
        for (i = 0; i < numthr; i++) {
                w[i].mats = mats;
                w[i].r = &ranges[i];
                pthread_create(&thr[i], NULL, (void *) &relax, (void *) &w[i]);
        }

        /**************************************************/

        printmat(mats);
        printf("\n");
        // handle signalling
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

                // threads waiting before they can finish
                // prevent them from starting again
                ready = 0;
                idle = 0;

                // allow them to finish
                pthread_mutex_lock(&mtx_finish);
                finish = 1;
                pthread_cond_broadcast(&cnd_finish);
                pthread_mutex_unlock(&mtx_finish);

                numits++;
        } while (check(mats, prec));

        printmat(mats);
        printf("\n");

        printf("Complete in %d iterations.\n", numits);


        /**************************************************/
        // free memory
        freemat(mats->imat, mats->size);
        freemat(mats->rmat, mats->size);
        free(mats);
        free(ranges);
        free(w);
        free(thr);
        return 0;
}
