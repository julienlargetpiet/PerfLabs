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

size_t *cartesian_indices(
    const size_t *lengths,
    size_t ndim,
    size_t *out_rows
) {
    size_t total = 0;

    if (!get_total_length(lengths, ndim, &total)) {
        return NULL;
    }

    *out_rows = total;

    if (total == 0) {
        return NULL;
    }

    if (ndim == 0) {
        return NULL;
    }

    if (total > SIZE_MAX / ndim) {
        return NULL;
    }

    size_t n_elems = total * ndim;

    if (n_elems > SIZE_MAX / sizeof(size_t)) {
        return NULL;
    }

    size_t *indices = malloc(n_elems * sizeof(*indices));
    if (indices == NULL) {
        return NULL;
    }

    size_t *cur_indices = calloc(ndim, sizeof(*cur_indices));
    if (cur_indices == NULL) {
        free(indices);
        return NULL;
    }

    size_t row = 0;
    size_t dim = ndim - 1;

    while (row < total) {
        const size_t row_idx = ndim * row;

        for (size_t i = 0; i < ndim; ++i) {
            indices[row_idx + i] = cur_indices[i];
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
    return indices;
}

void makeMatrix(Matrix2D *mat,
                const size_t *indices,
                const size_t *data[],
                const size_t nrows,
                const size_t ndim
               ) {

    size_t *vec = mat->data;

    for (size_t i = 0; i < nrows; ++i) {
        const size_t base_pos = i * ndim;
        for (size_t i2 = 0; i2 < ndim; ++i2) {
            vec[base_pos + i2] = data[i2][indices[base_pos + i2]];
        }
    }
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

    const size_t *data[] = {a, b, c, d, e};

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
        size_t rows = 0;
        size_t *indices = cartesian_indices(lengths, ndim, &rows);

        if (indices == NULL && rows != 0) {
            fprintf(stderr, "cartesian_indices failed during benchmark\n");
            return 1;
        }

        Matrix2D mat = matrix_create(rows, ndim);
        makeMatrix(&mat, 
                   indices, 
                   data,
                   rows,
                   ndim);

        sink += mat.data[0];
        sink += mat.data[rows * ndim - 1];

        free(mat.data);
        free(indices);
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


