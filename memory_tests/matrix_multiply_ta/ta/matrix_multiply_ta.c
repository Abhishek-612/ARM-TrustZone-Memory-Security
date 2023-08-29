// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) [Year], [Author]
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <matrix_multiply_ta.h>
#include <string.h>

TEE_Result TA_CreateEntryPoint(void)
{
    return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
	DMSG("Destroying Entry Point\n");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types, TEE_Param __unused params[4], void **session)
{
    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *session)
{
	DMSG("Closing Session\n");
}

static TEE_Result multiply_matrices(uint32_t param_types, TEE_Param params[4])
{
    DMSG("multiply_matrices reached!\n");
    // Ensure that the input parameters are of the correct type
    if (param_types != TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                                       TEE_PARAM_TYPE_MEMREF_INPUT,
                                       TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                       TEE_PARAM_TYPE_NONE))
    {
        return TEE_ERROR_BAD_PARAMETERS;
    }
    DMSG("Params are correct!\n");

    TEE_Param *in_matrix1 = &params[0];
    TEE_Param *in_matrix2 = &params[1];
    TEE_Param *out_matrix = &params[2];
    
    DMSG("out_matrix address - %p - %p\n",out_matrix, &params[2]);
    DMSG("out_matrix buffer: %p\n", out_matrix->memref.buffer);

    DMSG("MAT1 -> size - %u\tbuffer - %u\tuint - %u\n",in_matrix1->memref.size,*(uint32_t *)in_matrix1->memref.buffer,sizeof(uint32_t));
    DMSG("MAT2 -> size - %u\tbuffer - %u\tuint - %u\n",in_matrix2->memref.size,*(uint32_t *)in_matrix2->memref.buffer,sizeof(uint32_t));
    // Extract the matrix dimensions from the input parameters
    uint32_t cols1 = *(uint32_t *)in_matrix1->memref.buffer;
    uint32_t rows1 = in_matrix1->memref.size / ( sizeof(uint32_t) * cols1 );
    uint32_t cols2 = *(uint32_t *)in_matrix2->memref.buffer;
    uint32_t rows2 = in_matrix2->memref.size / ( sizeof(uint32_t) * cols2 );
    
    DMSG("Dimensions are correct!\n");
    DMSG("cols1 - %u \t rows1 - %u\n",cols1, rows1);
    DMSG("cols2 - %u \t rows2 - %u\n",cols2, rows2);

    // Check if the matrices are compatible for multiplication
    if (cols1 != rows2)
    {
        return TEE_ERROR_BAD_PARAMETERS;
    }
    
    DMSG("Matrices are compatible!\n");

    // Calculate the size of the resulting matrix
    uint32_t result_size = rows1 * cols2 * sizeof(uint32_t);
    DMSG("Size calculated!\n");

    // Allocate memory for the result matrix using TEE_Malloc
    uint32_t *result = TEE_Malloc(result_size, TEE_MALLOC_FILL_ZERO);
    if (!result)
    {
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    DMSG("Memory allocated for result matrix!\n");

    // Perform matrix multiplication
    uint32_t *matrix1 = (uint32_t *)in_matrix1->memref.buffer + 1; // Skip the first element (cols1)
    uint32_t *matrix2 = (uint32_t *)in_matrix2->memref.buffer + 1; // Skip the first element (cols2)
    //uint32_t *result = (uint32_t *)out_matrix->memref.buffer;
    
    DMSG("Matrix multiplication started!\n");

    for (uint32_t i = 0; i < rows1; ++i)
    {
        for (uint32_t j = 0; j < cols2; ++j)
        {
            uint32_t sum = 0;

            for (uint32_t k = 0; k < cols1; ++k)
            {
                sum += matrix1[i * cols1 + k] * matrix2[k * cols2 + j];
            }

            result[i * cols2 + j] = sum;
        }
    }
    DMSG("Answer calculated!\n");
    DMSG("result[0] = %d\n",result[0]);
    
    // Set the output parameters
    out_matrix->memref.size = result_size;
    TEE_MemMove(out_matrix->memref.buffer, result, out_matrix->memref.size);
    DMSG("%p\t%u\n",result, result_size);
    DMSG("Result: %d\n", ((uint32_t *)out_matrix->memref.buffer)[0]);
    DMSG("output buffer: %p\n", out_matrix);
    DMSG("Result buffer: %p\n", out_matrix->memref.buffer);
    DMSG("Result size: %u\n", out_matrix->memref.size);
    DMSG("Buffer sent to normal world!\n");
    

    return TEE_SUCCESS;
}

TEE_Result TA_InvokeCommandEntryPoint(void __unused *session, uint32_t cmd_id, uint32_t param_types, TEE_Param params[4])
{
    DMSG("TA Invoke reached!\n %d\n",cmd_id);
    switch (cmd_id)
    {
    case TA_MATRIX_MULTIPLY_CMD:
        return multiply_matrices(param_types, params);
    default:
        return TEE_ERROR_BAD_PARAMETERS;
    }
}

