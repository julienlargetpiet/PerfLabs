/* Headers. */
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define ARR_LEN(a) (sizeof(a) / sizeof((a)[0]))

typedef size_t uad;
typedef uint64_t u64;
typedef double f64;
typedef bool u1;

typedef const size_t Uad;
typedef const uint64_t U64;
typedef const double F64;
typedef const bool U1;

/* The least significant dimension is at the left. */
typedef struct Comb {
    const u64 *const *data;
    u64 *strides;
    u64 stride_len;
} Comb;

void create_comb(Comb *comb,
                 const u64 *const data[],
                 U64 tot_dim,
                 const u64 *len_arr)
{
    comb->data = data;
    comb->stride_len = tot_dim - 1;

    comb->strides = NULL;
    
    if (comb->stride_len > 0) {
        comb->strides = malloc(sizeof(u64) * comb->stride_len);
        if (comb->strides == NULL) {
            fprintf(stderr, "Allocation failed.\n");
            abort();
        }
    }

    u64 cur_comb = 1;

    for (u64 i = 0; i < comb->stride_len; i++) {
        cur_comb *= len_arr[i];
        comb->strides[i] = cur_comb;
    }
}

void destroy_comb(Comb *comb)
{
    free(comb->strides);
    comb->strides = NULL;
    comb->data = NULL;
    comb->stride_len = 0;
}

void get_comb(const Comb *restrict comb,
              u64 *restrict cur_comb,
              U64 comb_value)
{
    U64 *const *restrict data = comb->data;
    U64 *restrict strides = comb->strides;

    u64 rem = comb_value;

    for (u64 i = comb->stride_len; i > 0; --i) {
        U64 stride = strides[i - 1];
        U64 idx = rem / stride;

        cur_comb[i] = data[i][idx];

        rem -= idx * stride;
    }

    cur_comb[0] = data[0][rem];

}

static uint64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int main(void)
{
    u64 tim_stt = now_ns();

    U64 a[] = {0,  1,  2,  3,  4};
    U64 b[] = {10, 11, 12, 13, 14};
    U64 c[] = {20, 21, 22, 23, 24};
    U64 d[] = {30, 31, 32, 33, 34};
    U64 e[] = {40, 41, 42, 43, 44};

    const u64 *const data[] = {a, b, c, d, e};

    U64 len_arr[] = {
        ARR_LEN(a),
        ARR_LEN(b),
        ARR_LEN(c),
        ARR_LEN(d),
        ARR_LEN(e)
    };

    U64 arr_nbr = ARR_LEN(len_arr);

    U64 itr_nbr = 100000;
    volatile u64 sum = 0;

    for (u64 itr_idx = 0; itr_idx < itr_nbr; itr_idx++) {
        Comb comb_obj;

        create_comb(&comb_obj,
                    data,
                    arr_nbr,
                    len_arr);

        u64 *cur_comb_array = malloc(sizeof(u64) * arr_nbr);
        if (cur_comb_array == NULL) {
            fprintf(stderr, "Allocation failed.\n");
            abort();
        }

        for (size_t i = 0; i < (U64)pow(5, 4); ++i) {
            get_comb(&comb_obj, cur_comb_array, i);
            sum += cur_comb_array[0];
            sum += cur_comb_array[arr_nbr - 1];
        }

        free(cur_comb_array);
        destroy_comb(&comb_obj);
    }

    u64 tim_ttl = now_ns() - tim_stt;

    f64 ns_per_iter = (f64)tim_ttl / (f64)itr_nbr;

    printf("iterations:       %" PRIu64 "\n", itr_nbr);
    printf("elapsed:          %" PRIu64 " ns\n", tim_ttl);
    printf("ns / call:        %.2f\n", ns_per_iter);
    printf("sink %" PRIu64 "\n", sum);

    return 0;
}



