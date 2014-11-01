/*#include <getopt.h>*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct workRange {
    int start;
    int end;
};

/*
 * Print a matrix of given size to stdout
 */
void printMatrix(float **matrix, int size)
{
    int i, j;
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            // only print 2dp to save screen space
            if (i > 0 && j > 0 && i < size - 1 && j < size - 1)
            {
                // print inner matrix in different colour
                /*printf("\033[30;1m%.2f\033[0m\t", matrix[i][j]);*/
                printf("%f\t", matrix[i][j]);
            }
            else
            {
                printf("%f\t", matrix[i][j]);
            }
        }
        printf("\n");
    }
}


/*
 * Create a matrix of given size
 */
float** createMatrix(int size)
{
    float **matrix;

    // allocate memory for each row
    matrix = malloc(size * sizeof(float *));
    // allocate memory for each column
    int i;
    for (i = 0; i < size; i++)
    {
        matrix[i] = malloc(size * sizeof(float));
        /*if (matrix[i] == NULL) { fprintf(stderr, "out of memory\n"); }*/
    }
    return matrix;
}


/*
 * Initialise a matrix with values
 */
void initMatrix(float **matrix, int size, int *array)
{
    int i, j;
    int currentPosition = 0; // within the array
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            matrix[i][j] = array[currentPosition];
            currentPosition++;
        }
    }
}


/*
 * Create an array of a given length
 * populate with random ints
 */
int* createArray(int length)
{
    srand(time(NULL));
    int *array = malloc(length * sizeof(int));
    int i;
    for (i = 0; i < length; i++)
    {
        array[i] = rand() % 5;
    }
    return array;
}

/*
 * Destroy a matrix of given size
 */
void destroy(float **matrix, int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}


/*
 * Find the inner size of a matrix
 * really simple, but saves on confusion
 */
int innerSize(int size)
{
    return size - 2;
}


/*
 * 'Partition' the matrix so each thread has roughly equal work
 */
struct workRange* partitionMatrix(int size, int t)
{
    int inrSize = innerSize(size);
    int unallocatedRows = inrSize;
    int currentThread = 0;
    int i;

    int *workAllocation = malloc(inrSize * sizeof(int));
    for (i = 0; i < inrSize; i++) workAllocation[i] = 0;

    // while there are rows that have not been allocaded
    while (unallocatedRows > 0)
    {
        // give the current thread an extra row
        workAllocation[currentThread]++;
        unallocatedRows--;
        currentThread++; // next thread, loop to start if needed
        if (currentThread == t) currentThread = 0;
    }

    // work out start/end points
    struct workRange *ranges = malloc(t * sizeof(struct workRange));
    for (i = 0; i < t; i++)
    {
        if (i == 0)
        {
            ranges[i].start = 1;
            ranges[i].end = ranges[i].start + workAllocation[i];
        }
        else
        {
            ranges[i].start = ranges[i - 1].start + workAllocation[i - 1];
            ranges[i].end = ranges[i].start + workAllocation[i];
        }
    }
    free(workAllocation);
    return ranges;
}


/*
 * Relax. Calculate the averages for rows in a given range
 */
void relax(float **imatrix, float **rmatrix, int size, struct workRange wr)
{
    int i, j;
    float sum, avg;

    for (i = wr.start; i < wr.end; i++)
    {
        for (j = 1; j < size - 1; j++)
        {
            sum = imatrix[i - 1][j] // north
                + imatrix[i][j + 1] // east
                + imatrix[i + 1][j] // south
                + imatrix[i][j - 1];// west
            avg = sum / 4;
            rmatrix[i][j] = avg;
        }
    }
}


/*
 * Main
 * TODO:
 *  - Remove automatic generation of maitrces for final submission
 */
int main(int argc, char **argv)
{
    int size, threads, precision, arrayLength;
    int *array;
    int i;
    int arrayPassed = 0;

    if (argc < 4) // too few arguments
    {
        fprintf(stderr, "Error: Too few arguments\n");
        exit(1);
    }
    else if (argc > 4) // array is passed at CL
    {
        arrayPassed = 1;
        size = atoi(argv[1]);
        threads = atoi(argv[2]);
        precision = atoi(argv[3]);
        arrayLength = size * size;
        if (argc - arrayLength != 4 || size < 3)
        {
            fprintf(stderr, "Error: Bad array length\n");
            exit(1);
        }
        array = malloc(arrayLength * sizeof(int));
        for (i = 0; i < arrayLength; i++)
        {
            array[i] = atoi(argv[i + 4]);
        }
    }
    else // array not passed, generate randomly
    {
        size = atoi(argv[1]);
        threads = atoi(argv[2]);
        precision = atoi(argv[3]);
        arrayLength = size * size;
        array = createArray(arrayLength);
    }


    // print information to stdout, for debug purposes
    printf("Size: %dx%d\n", size, size);
    printf("Threads: %d\n", threads);
    printf("Precision: %d\n", precision);


    // Create initial matrix, with values
    float **imatrix = createMatrix(size);
    initMatrix(imatrix, size, array);
    // create result matrix with same values
    float **rmatrix = createMatrix(size);
    initMatrix(rmatrix, size, array);

    // Partition matrix so each thread works equally
    struct workRange *ranges = partitionMatrix(size, threads);
    // print thread allocation info, debug purposes
    for (i = 0; i < threads; i++)
    {
        printf("Thread %d of %d will work on rows %d to %d\n",
                i + 1,
                threads,
                ranges[i].start,
                ranges[i].end
                );
    }



    // free the used memory
    destroy(imatrix, size);
    destroy(rmatrix, size);
    free(array);
    free(ranges);
    return 0;
}
