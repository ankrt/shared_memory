/*#include <getopt.h>*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct range {
        int start;
        int end;
};

/*
 * Print a matrix of given size to stdout
 */
void printmat(float **mat, int size)
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
float** createmat(int size)
{
        int i;
        float **mat;
        mat = malloc(size * sizeof(float *));
        for (i = 0; i < size; i++) {
                mat[i] = malloc(size * sizeof(float));
                /*if (mat[i] == NULL) { fprintf(stderr, "out of memory\n"); }*/
        }
        return mat;
}


/*
 * Initialise a matrix with values
 */
void initmat(float **mat, int size, int *arr)
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
void freemat(float **mat, int size)
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
                        ranges[i].end = ranges[i].start + allocation[i];
                } else {
                        ranges[i].start = ranges[i - 1].start + allocation[i - 1];
                        ranges[i].end = ranges[i].start + allocation[i];
                }
        }
        free(allocation);
        return ranges;
}


/*
 * Relax. Calculate the averages for rows in a given range
 */
void relax(float **imat, float **rmat, int size, struct range r)
{
        int i, j;
        float sum, avg;

        for (i = r.start; i < r.end; i++) {
                for (j = 1; j < size - 1; j++) {
                        sum = imat[i - 1][j]
                                + imat[i][j + 1]
                                + imat[i + 1][j]
                                + imat[i][j - 1];
                        avg = sum / 4;
                        rmat[i][j] = avg;
                }
        }
}

int check(float **imat, float **rmat, int size, float prec)
{
        int i, j;
        float diff;
        int matching = 0;

        for (i = 0; i < size; i++) {
                for (j = 0; j < size; j++) {
                        diff = abs(imat[i][j] - rmat[i][j]);
                        if (diff < (1 / prec)) {
                                matching = 1;
                        } else {
                                matching = 0;
                        }
                }
        }
        return(matching);
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

        float **imat = createmat(size);
        float **rmat = createmat(size);
        initmat(imat, size, arr);
        initmat(rmat, size, arr);

        // Partition matrix so each thread works equally
        struct range *ranges = partmat(size, numthr);
        for (i = 0; i < numthr; i++) {
                printf("Thread %d of %d will work on rows %d to %d\n",
                        i + 1,
                        numthr,
                        ranges[i].start,
                        ranges[i].end
                      );
        }

        printmat(imat, size);
        relax(imat, rmat, size, ranges[0]);
        printf("\n");
        printmat(rmat, size);

        freemat(imat, size);
        freemat(rmat, size);
        free(arr);
        free(ranges);
        return 0;
}
