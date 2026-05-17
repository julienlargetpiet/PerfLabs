/* Headers. */
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>

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
    u64 tot_dim;
} Comb;

void create_comb(Comb *comb,
                 const u64 *const data[],
                 U64 tot_dim,
                 const u64 *len_arr)
{
    comb->data = data;
    comb->tot_dim = tot_dim;

    comb->strides = malloc(sizeof(u64) * tot_dim);
    if (comb->strides == NULL) {
        fprintf(stderr, "Allocation failed.\n");
        abort();
    }

    comb->strides[0] = 1;

    for (u64 i = 1; i < tot_dim; i++) {
        comb->strides[i] = comb->strides[i - 1] * len_arr[i - 1];
    }
}

void destroy_comb(Comb *comb)
{
    free(comb->strides);
    comb->strides = NULL;
    comb->data = NULL;
    comb->tot_dim = 0;
}

void get_comb(const Comb *comb,
              u64 *cur_comb,
              U64 comb_value)
{
    const u64 *const *data = comb->data;
    const u64 *strides = comb->strides;

    u64 rem = comb_value;

    for (u64 i = comb->tot_dim; i-- > 0;) {
        U64 stride = strides[i];
        U64 idx = rem / stride;

        cur_comb[i] = data[i][idx];

        rem -= idx * stride;
    }
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

        get_comb(&comb_obj, cur_comb_array, 133);

        for (u64 i = 0; i < arr_nbr; i++) {
            sum += cur_comb_array[i];
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



