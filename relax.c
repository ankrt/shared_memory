#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>


/*
 * Print program usage to the terminal
 */
void printUsage() {
    printf("Usage: rectangle -s size -t threads -p precision\n");
}


/*
 * Create a matrix of given size
 */
float** createMatrix(int size)
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

    /*
     * populate matrix with values
     * this will be removed as array will
     * eventually be passed directly to the
     * program
     */
    srand(time(NULL));
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            matrix[i][j] = (float) (rand() % 5);
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

    int opt = 0;
    int threads = -1, size = -1, precision = -1;

    // handle command line arguments
    while ((opt = getopt(argc, argv,"s:t:p:")) != -1) {
        switch (opt) {
            case 's' :
                size = atoi(optarg);
                break;
            case 't' :
                threads = atoi(optarg);
                break;
            case 'p' :
                precision = atoi(optarg);
                break;
            default:
                printUsage();
                exit(EXIT_FAILURE);
        }
    }
    if (size == -1 || threads == -1 || precision == -1) {
        printUsage();
        exit(EXIT_FAILURE);
    }

    float **matrix = createMatrix(size);
    printMatrix(matrix, size);

    // decide how to split up the matrix

    return 0;
}
