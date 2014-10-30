#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
/*#include <getopt.h>*/
/*#include <time.h>*/


/*
 * Print program usage to the terminal
 */
void printUsage() {
    printf("Usage: rectangle -s size -t threads -p precision\n");
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
            printf("%.2f\t", matrix[i][j]);
        }
        printf("\n");
    }
}


/*
 * Print the inner matrix
 * debug purposes
 */
void printInnerMatrix(float **matrix, int size)
{
    int i, j;
    for (i = 1; i < size - 1; i++)
    {
        for (j = 1; j < size - 1; j++)
        {
            printf("%.2f\t", matrix[i][j]);
        }
        printf("\n");
    }
}


/*
 * Main
 */
int main(int argc, char **argv) {

    if (argc < 4)
    {
        fprintf(stderr, "Error: Too few arguments\n");
        exit(1);
    }

    int size, threads, precision, arrayLength;

    size = atoi(argv[1]);
    threads = atoi(argv[2]);
    precision = atoi(argv[3]);
    arrayLength = size * size;

    printf("%d%s", argc - arrayLength, "\n");
    if (argc - arrayLength != 4)
    {
        fprintf(stderr, "Erorr: Size and array length do not match\n");
        exit(1);
    }

    // print information to stdout, for debug purposes
    printf("Size: %d by %d\n", size, size);
    printf("Threads: %d\n", threads);
    printf("Precision: %d\n", precision);

    int *array = malloc(arrayLength * sizeof(int));
    int i;
    for (i = 0; i < arrayLength; i++)
    {
        array[i] = atoi(argv[i + 4]);
    }

    float **matrix = createMatrix(size, array);
    printMatrix(matrix, size);
    free(array);
    destroyMatrix(matrix, size);

    return 0;
}
