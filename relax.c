/*#include <getopt.h>*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


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
                printf("\033[30;1m%.2f\033[0m\t", matrix[i][j]);
            }
            else
            {
                printf("%.2f\t", matrix[i][j]);
            }
        }
        printf("\n");
    }
}


/*
 * Create a matrix of given size
 */
float** createMatrix(int size, int *array)
{
    float **matrix;

    // allocate memory for each row
    matrix = malloc(size * sizeof(float *));
    /*if (matrix == NULL) { fprintf(stderr, "out of memory\n"); }*/

    // allocate memory for each column
    int i, j;
    for (i = 0; i < size; i++)
    {
        matrix[i] = malloc(size * sizeof(float));
        /*if (matrix[i] == NULL) { fprintf(stderr, "out of memory\n"); }*/
    }

    int count = 0;
    // populate the matrix with numbers from array
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            matrix[i][j] = array[count];
            count++;
        }
    }
    return matrix;
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
void destroyMatrix(float **matrix, int size)
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
int* partitionMatrix(float **matrix, int size, int t)
{
    int inrSize = innerSize(size);
    int unallocatedRows = inrSize;
    int currentThread = 0;
    int i;

    int *rowAllocation = malloc(inrSize * sizeof(int));
    for (i = 0; i < inrSize; i++) rowAllocation[i] = 0;

    // while there are rows that have not been allocaded
    while (unallocatedRows > 0)
    {
        // give the current thread an extra row
        rowAllocation[currentThread]++;
        unallocatedRows--;
        currentThread++; // next thread, loop to start if needed
        if (currentThread == t) currentThread = 0;
    }
    return rowAllocation;
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


    float **matrix = createMatrix(size, array);

    // Work out how to split the matrix so
    // each thread works on it ~equally
    int *rowAllocation = partitionMatrix(matrix, size, threads);
    for (i = 0; i < threads; i++)
    {
        printf("Thread %d of %d will work on %d rows\n",
                i + 1,
                threads,
                rowAllocation[i]);
    }
    /*printMatrix(matrix, size);*/
    destroyMatrix(matrix, size);
    free(rowAllocation);
    free(array);
    return 0;
}
