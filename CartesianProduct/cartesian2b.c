#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>

typedef struct Matrix2D {
    size_t nrows;
    size_t ncols;
    size_t *data;
} Matrix2D;

Matrix2D matrix_create(size_t nrows, 
                       size_t ncols
                      ) {
    Matrix2D mat;

    mat.nrows = nrows;
    mat.ncols = ncols;
    mat.data = malloc(nrows * ncols * sizeof(size_t));

    if (mat.data == NULL) {
        fprintf(stderr, "Allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    return mat;
}

size_t matrix_get(const Matrix2D *mat, 
                  const size_t row,
                  const size_t col) {
    return mat->data[row * mat->ncols + col];
}

bool get_total_length(
    const size_t *lengths,
    size_t ndim,
    size_t *out_total
) {
    size_t total = 1;

    for (size_t i = 0; i < ndim; ++i) {
        if (lengths[i] == 0) {
            *out_total = 0;
            return true;
        }

        if (total > SIZE_MAX / lengths[i]) {
            return false; // overflow protection
        }

        total *= lengths[i];
    }

    *out_total = total;
    return true;
}

void cartesian_product_matrix(
    Matrix2D *mat,
    const size_t *lengths,
    const size_t *data,
    size_t ndim,
    size_t nval
) {
    size_t total = 0;

    if (!get_total_length(lengths, ndim, &total)) {
        return;
    }

    if (total == 0) {
        return;
    }

    if (ndim == 0) {
        return;
    }

    if (total > SIZE_MAX / ndim) {
        return;
    }

    size_t n_elems = total * ndim;

    if (n_elems > SIZE_MAX / sizeof(size_t)) {
        return;
    }

    mat->nrows = total;
    mat->ncols = ndim;
    mat->data = malloc(n_elems * sizeof(size_t));

    size_t *actual_data = mat->data;

    if (actual_data == NULL) {
        return;
    }

    size_t *cur_indices = calloc(ndim, sizeof(*cur_indices));
    if (cur_indices == NULL) {
        free(actual_data);
        mat->data = NULL;
        mat->nrows = 0;
        mat->ncols = 0;
        return;
    }

    size_t row = 0;
    size_t dim = ndim - 1;

    while (row < total) {
        const size_t row_idx = ndim * row;

        for (size_t i = 0; i < ndim; ++i) {
            actual_data[row_idx + i] = data[nval * i + cur_indices[i]];
        }

        ++row;

        if (row == total) {
            break;
        }

        while (dim > 0 && cur_indices[dim] + 1 == lengths[dim]) {
            cur_indices[dim] = 0;
            --dim;
        }

        ++cur_indices[dim];
        dim = ndim - 1;
    }

    free(cur_indices);
}

static uint64_t now_ns(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int main(void) {

    const size_t a[] = {0,  1,  2,  3,  4};  
    const size_t b[] = {10, 11, 12, 13, 14};                         
    const size_t c[] = {20, 21, 22, 23, 24};                         
    const size_t d[] = {30, 31, 32, 33, 34};                         
    const size_t e[] = {40, 41, 42, 43, 44};                         

    size_t data[25];
    
    memcpy(data + 0,  a, sizeof(a));
    memcpy(data + 5,  b, sizeof(b));
    memcpy(data + 10, c, sizeof(c));
    memcpy(data + 15, d, sizeof(d));
    memcpy(data + 20, e, sizeof(e));

    const size_t lengths[] = {
        sizeof(a) / sizeof(a[0]),
        sizeof(b) / sizeof(b[0]),
        sizeof(c) / sizeof(c[0]),
        sizeof(d) / sizeof(d[0]),
        sizeof(e) / sizeof(e[0])
    };

    const size_t ndim = sizeof(lengths) / sizeof(lengths[0]);

    const size_t iterations = 100000;

    uint64_t start = now_ns();
    volatile size_t sink = 0;

    for (size_t i = 0; i < iterations; ++i) {
        Matrix2D mat;
        cartesian_product_matrix(&mat,
                                 lengths, 
                                 data,
                                 ndim,
                                 lengths[0]
        );

        sink += mat.data[0];
        sink += mat.data[mat.nrows * ndim - 1];

        free(mat.data);
    }

    uint64_t end = now_ns();

    uint64_t elapsed_ns = end - start;

    double ns_per_iter = (double)elapsed_ns / (double)iterations;

    printf("iterations:       %zu\n", iterations);
    printf("elapsed:          %" PRIu64 " ns\n", elapsed_ns);
    printf("ns / call:        %.2f\n", ns_per_iter);

    printf("sink %zu\n", sink);

    return 0;
}



