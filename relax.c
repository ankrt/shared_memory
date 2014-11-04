/*#include <getopt.h>*/
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int thrcycles = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv_thrcycle = PTHREAD_COND_INITIALIZER;
pthread_cond_t cv_continue = PTHREAD_COND_INITIALIZER;

// thread has completed one cycle, increment counter
void incthrcycles() {
        pthread_mutex_lock(&mtx);
        printf("incthrcycles\n");
        thrcycles++;
        pthread_cond_signal(&cv_thrcycle);
        pthread_mutex_unlock(&mtx);
}

// wait until t threads have signalled
void allthrcycled(int t) {
        pthread_mutex_lock(&mtx);
        while (thrcycles < t)
                pthread_cond_wait(&cv_thrcycle, &mtx);
        pthread_mutex_unlock(&mtx);
}


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
                arr[i] = rand() % 5;
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
        struct work *w;
        w = (struct work *) ptr;

        int i, j;
        double sum, avg;
        sleep(2);
        printf("lol i'm a thread\n");
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
        incthrcycles();

        return NULL;
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
        for (i = 0; i < numthr; i++) {
                w[i].mats = mats;
                w[i].r = &ranges[i];
        }

        // thread variables
        pthread_t *thr = malloc(numthr * sizeof(pthread_t));
        // start all the threads
        for (i = 0; i < numthr; i++) {
                pthread_create(
                                &thr[i],
                                NULL,
                                (void *) &relax,
                                (void *) &w[i]);
        }
        // handle signalling
        allthrcycled(numthr);
        printf("All threads have completed an iteration\n");

        for (i = 0; i < numthr; i++) {
                pthread_exit(&thr[i]);
        }


        freemat(mats->imat, mats->size);
        freemat(mats->rmat, mats->size);
        free(mats);
        free(ranges);
        free(w);
        free(thr);
        return 0;
}
