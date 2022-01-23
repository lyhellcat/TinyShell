#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0

const int DIM = 1024;
const int MAX_VALUE = 20;


void multiply(const int dim, const int *const a,
              const int* const b, int * c) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            c[i * dim + j] = 0;
            for (int k = 0; k < dim; k++) {
                c[i * dim + j] += a[i * dim + k] * b[k * dim + j];
            }
        }
    }
}

void multiply_transpose(const int dim, const int * const a,
                        const int * const b_t, int * c) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            c[i * dim + j] = 0;
            for (int k = 0; k < dim; k++) {
                c[i * dim + j] += a[i * dim + k] * b_t[j * dim + k];
            }
        }
    }
}

void transpose(const int dim, int * m) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < i; j++) {
            int tmp = m[i * dim + j];
            m[i * dim + j] = m[j * dim + i];
            m[j * dim + i] = tmp;
        }
    }
}

void print(const int dim, int *m) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            printf("%d ", m[i * dim + j]);
        }
        puts("");
    }
    puts("");
}

void init(const int dim, int *m) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            m[i * dim + j] = rand() % MAX_VALUE;
        }
    }
}

int verify(const int dim, const int * const c1, const int * const c2) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            if (c1[i * dim + j] != c2[i * dim + j])
                return FALSE;
        }
    }
    return TRUE;
}

struct timeval run_and_time(
    void (* mult_func)(const int, const int * const, const int * const, int *),
    const int dim,
    const int * const a,
    const int * const b,
    int * c) {
        struct timeval start, end, delta;
        gettimeofday(&start, NULL);
        mult_func(dim, a, b, c);
        gettimeofday(&end, NULL);
        delta.tv_sec = end.tv_sec - start.tv_sec;
        delta.tv_usec = (end.tv_usec - start.tv_usec + 1000000) % 1000000;

        return delta;
}

void run_and_test(const int dim) {
    int *a = malloc(sizeof(int) * DIM * DIM);
    int *b = malloc(sizeof(int) * DIM * DIM);
    int *c1 = malloc(sizeof(int) * DIM * DIM);
    int *c2 = malloc(sizeof(int) * DIM * DIM);

    init(DIM, a);
    init(DIM, b);
    struct timeval time_elapese_1 = run_and_time(&multiply, DIM, a, b, c1);
    transpose(DIM, b);
    struct timeval time_elapese_2 = run_and_time(&multiply_transpose, DIM, a, b, c2);

    if (verify(DIM, c1, c2) == FALSE) {
        printf("Find bug!\n");
        return;
    }
    puts("Results agree.");
    printf("Standard multipicatiopn: %ld seconds. %ld microseconds\n",
        time_elapese_1.tv_sec, time_elapese_1.tv_usec);
    printf("Multiplication with transpose: %ld seconds. %ld microseconds\n",
           time_elapese_2.tv_sec, time_elapese_2.tv_usec);

    free(a);
    free(b);
    free(c1);
    free(c2);
}

int main() {
    run_and_test(DIM);

    return EXIT_SUCCESS;
}
