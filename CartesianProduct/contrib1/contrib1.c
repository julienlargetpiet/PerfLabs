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

/*
 * Return the current time.
 */
static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

/*
 * Replicate the content of @src (of size src_siz)
 * until @rep_nbr replicas exist in memory.
 * A @rep_nbr of 1 does not replicate anything.
 * Copies using exponential sizes.
 */
static inline void memrep(
        void *const src,
        U64 src_siz,
        U64 rep_nbr
)
{
        u64 rep_don = 1;
        u64 rep_wid = 1;
        while (rep_don < rep_nbr) {
                assert(rep_don == rep_wid);

                /* Adjust the size for the last iteration. */
                U64 rep_rem = rep_nbr - rep_don;
                u64 rep_siz = (rep_rem < rep_wid) ? rep_rem : rep_wid;

                /* Determine the copy dest. */
                u64 *const dst = src + rep_don * src_siz;
                memcpy(dst, src, rep_siz * src_siz);

                /* Update status. */
                rep_don += rep_siz;
                rep_wid <<= 1;
                assert(rep_don <= rep_nbr);
        }
        assert(rep_don == rep_nbr);
}

/*
 * Entrypoint.
 */
int main(void) {
    u64 tim_stt = now_ns();

        /* Test data. */
    U64 a[] = {0,  1,  2,  3,  4};
    U64 b[] = {10, 11, 12, 13, 14};
    U64 c[] = {20, 21, 22, 23, 24};
    U64 d[] = {30, 31, 32, 33, 34};
    U64 e[] = {40, 41, 42, 43, 44};
    U64 *const dat[] = {a, b, c, d, e};
    U64 len_arr[] = {
                ARR_LEN(a),
                ARR_LEN(b),
                ARR_LEN(c),
                ARR_LEN(d),
                ARR_LEN(e)
    };
    U64 arr_nbr = ARR_LEN(len_arr);

    /* Determine the carterian product size. */
    u64 combination_nbr = 1;
    for (u64 arr_idx = arr_nbr; arr_idx--;) {
            combination_nbr *= len_arr[arr_idx];
    }

    /* Allocate a matrix to contain all elements.
     * Layout :
     * [0] : elm[0].a
     * [1] : elm[1].a
     * ...
     * [combination_nbr] : elm[0].b
     * [combination_nbr + 1] : elm[1].b
     * ... and so on.
     */
    U64 dst_nbr = combination_nbr * arr_nbr; // combinaisons possibles * taile de chaque combinaison
    u64 *dst_elms = malloc(sizeof(u64) * dst_nbr);

        /* Compute the cartesian product. */
    U64 itr_nbr = 100000;
    volatile u64 sum = 0;
    for (volatile u64 itr_idx = 0; itr_idx < itr_nbr; itr_idx++) {

                /* Generate over each column of the elements array.
                 * This generates evert final elements's @arr_idx-th component. */
                u64 elm_dup_nbr = 1;
                u64 seq_dup_nbr = combination_nbr;
                for (u64 arr_idx = 0; arr_idx < arr_nbr; arr_idx++) {

                        /* The cartesian product duplicates elements
                         * of the base array a certain number of times
                         * (total product of array sizes before (<) this index)
                         * and then duplicates the generated result a certain
                         * number of times (total product of array sizes after (>) this index. */
                        u64 len = len_arr[arr_idx];
                        assert(!(seq_dup_nbr % len)); // on vérifie que c'est bien divisible
                        seq_dup_nbr /= len; // ici, on a donc le nombre de répétitions de la dupplication de la valeur

                        /* Start of this sequence. -> first combination value at Index arr_idx */
                        u64 *const stt = dst_elms + combination_nbr * arr_idx; 

                        /* First, generate the sequence by writing and
                         * replicating elements. */
                        u64 *dup_stt = stt;
                        for (u64 elm_idx = 0; elm_idx < len; elm_idx++) {
                                U64 val = dat[arr_idx][elm_idx];
                                *dup_stt = val;
                                if (elm_dup_nbr > 1) memrep(dup_stt, sizeof(u64), elm_dup_nbr);
                                dup_stt += elm_dup_nbr;
                        }

                        /* Update the next dup number, used below to
                         * represent the initial sequence size. */
                        elm_dup_nbr *= len;

                        /* Then, replicate the sequence. */
                        memrep(stt, sizeof(u64) * elm_dup_nbr, seq_dup_nbr);

                }
                assert(elm_dup_nbr == combination_nbr);
                assert(seq_dup_nbr == 1);

        sum += dst_elms[0];
        sum += dst_elms[dst_nbr - 1];

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

    /* Cleanup. */
    free(dst_elms);

        /* Log. */
    f64 ns_per_iter = (f64)tim_ttl / (f64) itr_nbr;
    printf("iterations:       %zu\n",  itr_nbr);
    printf("elapsed:          %lu ns\n", tim_ttl);
    printf("ns / call:        %.2f\n", ns_per_iter);
    printf("sink %zu\n", sum);

    return 0;
}
