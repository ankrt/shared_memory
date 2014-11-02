/*#include <getopt.h>*/
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*
 * TIL barriers are not implemented in OSX
 */
pthread_barrier_t barrier;

struct range {
        int start;
        int end;
};

struct matrices {
        double **imat;
        double **rmat;
        int size;
};

void printmat(double **mat, int size)
{
        int i, j;
        for (i = 0; i < size; i++) {
                for (j = 0; j < size; j++) {
                        printf("%f\t", mat[i][j]);
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
void relax(struct matrices *mats, struct range r)
{
        int i, j;
        double sum, avg;

        for (i = r.start; i < r.end; i++) {
                for (j = 1; j < mats->size - 1; j++) {
                        sum = mats->imat[i - 1][j]
                                + mats->imat[i][j + 1]
                                + mats->imat[i + 1][j]
                                + mats->imat[i][j - 1];
                        avg = sum / 4;
                        mats->rmat[i][j] = avg;
                }
        }
        pthread_barrier_wait(&barrier);
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

        printf("Size: %dx%d\n", size, size);
        printf("numthr: %d\n", numthr);
        printf("prec: %d\n", prec);

        struct matrices *mats = malloc(sizeof(struct matrices));
        mats->imat = createmat(size);
        mats->rmat = createmat(size);
        mats->size = size;
        initmat(mats->imat, mats->size, arr);
        initmat(mats->rmat, mats->size, arr);
        free(arr);

        // Partition matrix so each thread works equally
        struct range *ranges = partmat(size, numthr);
        for (i = 0; i < numthr; i++) {
                printf("Thread %d of %d will work on rows %d <= r < %d\n",
                        i + 1,
                        numthr,
                        ranges[i].start,
                        ranges[i].end
                      );
        }

        do {
                pthread_barrier_init(&barrier, NULL, numthr);
                for (i = 0; i < numthr; i++)
                {
                        pthread_create(NULL,
                                        NULL,
                                        relax(mats, ranges[i]),
                                        NULL);
                }
                pthread_barrier_wait(&barrier);
                pthread_barrier_wait
                swap(mats);
        } while (check(mats, prec));
        pthread_exit(NULL);



        freemat(mats->imat, mats->size);
        freemat(mats->rmat, mats->size);
        free(mats);
        free(ranges);
        return 0;
}
