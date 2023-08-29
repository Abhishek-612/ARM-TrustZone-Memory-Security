// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) [Year], [Author]
 */

#include <err.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tee_client_api.h>
#include <matrix_multiply_ta.h>

#define DEBUG_MODE	0

static void debug(char *msg) 
{
	if (DEBUG_MODE)
		printf("DEBUG: %s\n",msg);
}

static void print_matrix(uint32_t *matrix, uint32_t rows, uint32_t cols, int buff)
{
    for (uint32_t i = 0; i < rows; ++i)
    {
        for (uint32_t j = 0; j < cols; ++j)
        {
            printf("%" PRIu32 " ", matrix[i * cols + j + buff]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    TEEC_Result res;
    TEEC_Context context;
    TEEC_Session session;
    
    TEEC_Operation op = { 0 };
    
    const TEEC_UUID ta_uuid = TA_MATRIX_MULTIPLY_UUID;

    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <rows1> <cols1> <rows2> <cols2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    uint32_t rows1 = atoi(argv[1]);
    uint32_t cols1 = atoi(argv[2]);
    uint32_t rows2 = atoi(argv[3]);
    uint32_t cols2 = atoi(argv[4]);

    // Allocate memory for the matrices
    // Remove the extra element for storing the number of columns
    uint32_t *matrix1 = malloc((rows1 * cols1 + 1) * sizeof(uint32_t));
    uint32_t *matrix2 = malloc((rows2 * cols2 + 1) * sizeof(uint32_t));

    uint32_t *result = NULL;

    if (!matrix1 || !matrix2)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    matrix1[0] = cols1;
    for (uint32_t i = 1; i <= rows1 * cols1; ++i)
    {
        matrix1[i] = rand() % 10;
    }

    matrix2[0] = cols2;
    for (uint32_t i = 1; i <= rows2 * cols2; ++i)
    {
        matrix2[i] = rand() % 10;
    }

    printf("\nMatrix 1:\n");
    print_matrix(matrix1, rows1, cols1, 1);

    printf("Matrix 2:\n");
    print_matrix(matrix2, rows2, cols2, 1);

    res = TEEC_InitializeContext(NULL, &context);
    if (res != TEEC_SUCCESS)
    {
        fprintf(stderr, "TEEC_InitializeContext failed with code 0x%x\n", res);
        goto cleanup;
    }

    res = TEEC_OpenSession(&context, &session, &ta_uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &op);

    if (res != TEEC_SUCCESS)
    {
        fprintf(stderr, "TEEC_OpenSession failed with code 0x%x\n", res);
        goto cleanup_teec;
    }

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_MEMREF_TEMP_OUTPUT,
                                     TEEC_NONE);
                                     
    // Pass matrix1 and matrix2 as input parameters
    op.params[0].tmpref.buffer = matrix1;
    op.params[0].tmpref.size = (rows1 * cols1) * sizeof(uint32_t);
    
    op.params[1].tmpref.buffer = matrix2;
    op.params[1].tmpref.size = (rows2 * cols2) * sizeof(uint32_t);
    
    op.params[2].tmpref.size = (rows1 * cols2) * sizeof(uint32_t);
    op.params[2].tmpref.buffer = malloc(op.params[2].tmpref.size);
	if (!op.params[2].tmpref.buffer)
		err(1, "Cannot allocate out buffer of size %zu",
		    op.params[2].tmpref.size);
		    

    res = TEEC_InvokeCommand(&session, TA_MATRIX_MULTIPLY_CMD, &op, NULL);
    if (res != TEEC_SUCCESS)
    {
        fprintf(stderr, "TEEC_InvokeCommand failed with code 0x%x\n", res);
        goto cleanup_session;
    }
    
    
    // Check if the result buffer is NULL
    if (!op.params[2].tmpref.buffer)
    {
    	fprintf(stderr, "Result buffer is NULL\n");
    	goto cleanup_session;
    }

    result = (uint32_t *)op.params[2].tmpref.buffer;
    printf("\nResult:\n");
    print_matrix(result, rows1, cols2, 0);
    debug("Reached 1");

cleanup_session:
    TEEC_CloseSession(&session);
    debug("Reached 2");

cleanup_teec:
    TEEC_FinalizeContext(&context);
    debug("Reached 3");

cleanup:
    if (matrix1)
    	free(matrix1);
    debug("Reached 4");
    
    if (matrix2)
    	free(matrix2);
    debug("Reached 5");
    
    if (result)
        free(result);
    debug("Reached 6");
    

    return (res == TEEC_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}

