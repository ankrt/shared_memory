#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/*
 * Print the matrix to stdout
 */
void printMatrix(float **matrix, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("%.2f\t", matrix[i][j]);
        }
        printf("\n");
    }
}


/*
 * Make every element in the matrix 0
 */
void zeroMatrix(float **matrix, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            matrix[i][j] = (float) 0;
        }
    }
}


/*
 * Assign values to each element in
 * a size*size matrix. Each value
 * is randomly generated to be within
 * a given range, 0<=n<range
 */
void populateMatrix(float **matrix, int size, int range)
{
    srand(time(NULL));
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            matrix[i][j] = (float) (rand() % range);
        }
    }
}


/*
 * Free the memory that has been allocated
 * to a size*size matrix
 */
void destroyMatrix(float **matrix, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            matrix[i][j] = (float) 0;
        }
    }
}


/*
 * Generate size*size matrix of
 * numbers in the range, range
 */
int generateMatrix(int size, int range)
{

    float **matrix;
    // allocate memory for each row
    matrix = malloc(size * sizeof(float *));
    if (matrix == NULL)
    {
        fprintf(stderr, "out of memory\n");
        return 1;
    }
    // then, for each row, allocate
    // memory for each column
    for (int i = 0; i < size; i++)
    {
        matrix[i] = malloc(size * sizeof(float));
        if (matrix[i] == NULL)
        {
            fprintf(stderr, "out of memory\n");
            return 1;
        }
    }

    // assign values to matrix
    populateMatrix(matrix, size, range);

    // print matrix
    printMatrix(matrix, size);

    // deallocate memory of matrix
    destroyMatrix(matrix, size);

    return 0;
}

/*
 * Main
 */
int main(int argc, char** argv)
{
    generateMatrix(10, 2);
    return 0;
}
