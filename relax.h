#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Encapsulate working range inside a struct
struct range {
        int start;
        int end;
};

// Encapsulate working matrices inside struct
struct matrices {
        double **imat;
        double **rmat;
        int size;
};

/*
 * Wrap up matrices and range into one struct
 * this will then be passed to a thread
 */
struct work {
        struct matrices *mats;
        struct range *r;
        double tolerance;
};

/*
 * Create an array and fill with random values
 */
int* createrandom(int length)
{
        int i;
        int *arr = malloc(length * sizeof(int));
        srand(time(NULL));
        for (i = 0; i < length; i++) {
                //arr[i] = rand() % 10;
                arr[i] = rand() % 2;
        }
        return arr;
}

/*
 * Create L-matrix
 */
int * createl(int size)
{
        int i;
        int length = size * size;
        int *arr = malloc(length * sizeof(int));
        for (i = 0; i < length; i++) {
                if (i < size || i % size == 0) {
                        arr[i] = 1;
                } else {
                        arr[i] = 0;
                }
        }
        return arr;
}

/*
 * Create an array and fill with values from argv
 */
int* createarr(int argc, char **argv, int length) {
        int i;
        int offset = argc - length;
        int *arr = malloc(length * sizeof(int));
        for (i = 0; i < length; i++) {
                arr[i] = atoi(argv[i + offset]);
        }
        return arr;
}

/*
 * Create an empty square matrix of a particular size
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
 * Initialise an empty matrix with values
 */
void fillmat(double **mat, int size, int *arr)
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
 * Print matrix to stdout
 */
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
 * Initialise structs with values
 */
void initmats(struct matrices *mats, int *arr,  int size)
{
        mats->imat = createmat(size);
        mats->rmat = createmat(size);
        mats->size = size;
        fillmat(mats->imat, mats->size, arr);
        fillmat(mats->rmat, mats->size, arr);
        free(arr);
}

/*
 * Free a matrix from memory
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
 * Return the inner size of a matrix
 * really simple, but saves on confusion
 */
int inrsize(int size)
{
        return size - 2;
}

/*
 * Partition the matrix into roughly equal chunks of rows
 * this will allow each thread to work on an approximately
 * even amount of data
 */
struct range* partmat(int size, int numthr)
{
        // split into equal chunks + a remainder
        // each thread gets a chunk, assign one of
        // the remaining rows to each thread in turn
        // until there are no more left to give out
        int isize = inrsize(size);
        int chunksize = isize / numthr;
        int remainder = isize % numthr;
        int i;

        // give each thread one of the equal sized chunks
        int *allocation = malloc(numthr * sizeof(int));
        for (i = 0; i < numthr; i++) {
                allocation[i] = chunksize;
        }

        int curthread = 0;
        while (remainder > 0) {
                allocation[curthread]++;
                remainder--;
                if (curthread == numthr) curthread = 0;
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
