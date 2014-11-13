#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

//// Encapsulate properties inside a struct
//struct progvars {
        //int size;
        //int numthr;
        //int precision;
        //int lenarr;

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
 * Decide what to do with the arguments that
 * are passed to the program
 */
//struct *progvars handleargs(int argc, char **argv) {
        //struct progvars *pv = malloc(sizeof(struct progvars));
        //if (argc < 4) {
                //fprintf(stderr, "Error: Too few arguments\n");
                //exit(1);
        //} else {

//}



/*
 * Create an array and fill with random values
 */
int* createrandom(int length)
{
        int i;
        int *arr = malloc(length * sizeof(int));
        //srand(time(NULL));
        for (i = 0; i < length; i++) {
                //arr[i] = rand() % 10;
                arr[i] = rand();
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
