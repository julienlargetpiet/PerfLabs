/* Headers. */
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>

/* Array length. */
#define ARR_LEN(a) (sizeof(a) / sizeof(a[0]))

/* Types + their const equivalents. */
typedef size_t uad;
typedef uint64_t u64;
typedef double f64;
typedef bool u1;
typedef const size_t Uad;
typedef const uint64_t U64;
typedef const double F64;
typedef const bool U1;

/* Abort if a condition is not met. */
#define assert(cdt) ({if (!(cdt)) {printf("%s:%d : assert(%s) failed.\n", __FILE__, __LINE__, #cdt);abort();}})

static inline void memrep(void *src, 
                          U64 src_siz, 
                          U64 rep_nbr)
{
    unsigned char *base = src;
    u64 rep_don = 1;

    while (rep_don < rep_nbr) {
        u64 rep_rem = rep_nbr - rep_don;
        u64 rep_cpy = rep_don < rep_rem ? rep_don : rep_rem;

        memcpy(base + rep_don * src_siz,
               base,
               rep_cpy * src_siz);

        rep_don += rep_cpy;
    }
}

static inline u1 u64_is_repeated_byte(u64 x)
{
    return x == ((x & 0xffu) * 0x0101010101010101ull);
}

static inline void fill_u64(u64 *dst, U64 value, U64 count)
{
    if (count == 0) {
        return;
    }

    if (u64_is_repeated_byte(value)) {
        memset(dst, (int)(value & 0xffu), count * sizeof(u64));
        return;
    }

    if (count <= 64) {
        for (u64 i = 0; i < count; i++) {
            dst[i] = value;
        }
        return;
    }

    dst[0] = value;

    u64 done = 1;

    while (done < count) {
        u64 rem = count - done;
        u64 cpy = done < rem ? done : rem;

        memcpy(dst + done,
               dst,
               cpy * sizeof(u64));

        done += cpy;
    }
}

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int main(void) {

    // Warning - To maximize what is copied over memcpy, if we have lists with diferent dimensions
    // we ask to sort the input lists from largest to smallest

    u64 tim_stt = now_ns();

        /* Test data. */
    U64 a[] = {0,  1,  2,  3,  4};
    U64 b[] = {10, 11, 12, 13, 14};
    U64 c[] = {20, 21, 22, 23, 24};
    U64 d[] = {30, 31, 32, 33, 34};
    U64 e[] = {40, 41, 42, 43, 44};
    U64 *const data[] = {a, b, c, d, e};
    U64 len_arr[] = {
                ARR_LEN(a),
                ARR_LEN(b),
                ARR_LEN(c),
                ARR_LEN(d),
                ARR_LEN(e)
    };
    U64 arr_nbr = ARR_LEN(len_arr);

    u64 combination_nbr = 1;
    for (u64 arr_idx = arr_nbr; arr_idx--;) {
            combination_nbr *= len_arr[arr_idx];
    }

    //U64 dst_nbr = combination_nbr * arr_nbr; // combinaisons possibles * taile de chaque combinaison
    //u64 *dst_elms = malloc(sizeof(u64) * dst_nbr);

    U64 itr_nbr = 100000;
    volatile u64 sum = 0;
    for (volatile u64 itr_idx = 0; itr_idx < itr_nbr; itr_idx++) {

        U64 dst_nbr = combination_nbr * arr_nbr; // combinaisons possibles * taile de chaque combinaison
        //u64 *dst_elms = malloc(sizeof(u64) * dst_nbr);

        u64 *dst_elms;
        if (posix_memalign((void **)&dst_elms, 64, sizeof(u64) * dst_nbr) != 0) {
            abort();
        }

        u64 elm_dup_nbr = 1;
        u64 seq_dup_nbr = combination_nbr;

        for (u64 arr_idx = 0; arr_idx < arr_nbr; arr_idx++) {
        
            U64 len = len_arr[arr_idx];
        
            seq_dup_nbr /= len;
        
            u64 *seq_start = dst_elms + combination_nbr * arr_idx;
            u64 *stt = seq_start;
        
            const u64 *cur_data = data[arr_idx];
       
            for (u64 elem_idx = 0; elem_idx < len; elem_idx++) {
                U64 cur_value = cur_data[elem_idx];

                fill_u64(stt, cur_value, elm_dup_nbr);
        
                stt += elm_dup_nbr;
            }

            u64 sequence_len = elm_dup_nbr * len;
        
            memrep(seq_start, 
                   sizeof(u64) * sequence_len, 
                   seq_dup_nbr);
        
            elm_dup_nbr = sequence_len;
        }

        sum += dst_elms[0];
        sum += dst_elms[dst_nbr - 1];

        free(dst_elms);

    }

        /* Get the current time. */
    u64 tim_ttl = now_ns() - tim_stt;

    ///* Print. */
    //for (u64 elm_idx = 0; elm_idx < combination_nbr; elm_idx++) {
    //        for (u64 arr_idx = 0; arr_idx < arr_nbr; arr_idx++) {
    //                printf("%lu ", dst_elms[arr_idx * combination_nbr + elm_idx]);
    //        }
    //        printf("\n");
    //}

    //free(dst_elms);

    f64 ns_per_iter = (f64)tim_ttl / (f64) itr_nbr;
    printf("iterations:       %zu\n",  itr_nbr);
    printf("elapsed:          %lu ns\n", tim_ttl);
    printf("ns / call:        %.2f\n", ns_per_iter);
    printf("sink %zu\n", sum);

    return 0;
}


