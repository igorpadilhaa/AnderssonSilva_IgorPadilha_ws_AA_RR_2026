#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>
#include <stdarg.h>

// --- LOGGER ---
FILE *log_file = NULL;
void init_logger() {
    log_file = fopen("benchmark.log", "a");
}
void close_logger() {
    if (log_file) fclose(log_file);
}
void logger(const char *level, const char *format, ...) {
    time_t now; time(&now);
    struct tm *local = localtime(&now);
    char time_str[64]; strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);
    va_list args1, args2; va_start(args1, format); va_copy(args2, args1);
    printf("[%s] [%s] ", time_str, level); vprintf(format, args1); printf("\n");
    if (log_file) {
        fprintf(log_file, "[%s] [%s] ", time_str, level); vfprintf(log_file, format, args2); fprintf(log_file, "\n"); fflush(log_file);
    }
    va_end(args1); va_end(args2);
}

// --- FUNÇÃO LINEAR O(n) ---
void fib_linear(mpz_t result, int n) {
    if (n <= 1) { mpz_set_ui(result, n); return; }
    mpz_t a, b, temp;
    mpz_inits(a, b, temp, NULL);
    mpz_set_ui(a, 0); mpz_set_ui(b, 1);
    for (int i = 2; i <= n; i++) {
        mpz_add(temp, a, b); mpz_set(a, b); mpz_set(b, temp);
    }
    mpz_set(result, b);
    mpz_clears(a, b, temp, NULL);
}

// --- FUNÇÃO FAST DOUBLING O(log n) ---
void fib_fast_doubling_pair(mpz_t f_n, mpz_t f_n_plus_1, int n) {
    if (n == 0) { mpz_set_ui(f_n, 0); mpz_set_ui(f_n_plus_1, 1); return; }
    mpz_t f_k, f_k_plus_1, t1, t2;
    mpz_inits(f_k, f_k_plus_1, t1, t2, NULL);
    
    fib_fast_doubling_pair(f_k, f_k_plus_1, n / 2);
    
    mpz_mul_ui(t1, f_k_plus_1, 2); mpz_sub(t1, t1, f_k);
    mpz_mul(t2, f_k, t1); 
    
    mpz_t f_k_sq, f_k_plus_1_sq;
    mpz_inits(f_k_sq, f_k_plus_1_sq, NULL);
    mpz_mul(f_k_sq, f_k, f_k); mpz_mul(f_k_plus_1_sq, f_k_plus_1, f_k_plus_1);
    mpz_add(t1, f_k_sq, f_k_plus_1_sq);
    
    if (n % 2 == 0) {
        mpz_set(f_n, t2); mpz_set(f_n_plus_1, t1);
    } else {
        mpz_set(f_n, t1); mpz_add(f_n_plus_1, t1, t2); 
    }
    mpz_clears(f_k, f_k_plus_1, t1, t2, f_k_sq, f_k_plus_1_sq, NULL);
}

void fib_fast_doubling(mpz_t result, int n) {
    mpz_t temp; mpz_init(temp);
    fib_fast_doubling_pair(result, temp, n);
    mpz_clear(temp);
}

// --- MAIN ---
int main() {
    init_logger();
    logger("INFO", "Iniciando Benchmark: Linear vs Fast Doubling");

    FILE *jsonl_file = fopen("resultados_otimizados.jsonl", "w");
    if (!jsonl_file) return EXIT_FAILURE;

    int tamanhos[] = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};
    int total_iteracoes = 13;

    for (int i = 0; i < sizeof(tamanhos) / sizeof(int); i++) {
        int n = tamanhos[i];
        logger("INFO", "Bateria Otimizada para N = %d", n);

        for (int iter = 1; iter <= total_iteracoes; iter++) {
            mpz_t res_linear, res_fast;
            mpz_inits(res_linear, res_fast, NULL);
            struct timespec start, end;
            long long elapsed_ns; double elapsed_sec;
            char *res_str;

            // --- Teste Linear ---
            clock_gettime(CLOCK_MONOTONIC, &start);
            fib_linear(res_linear, n);
            clock_gettime(CLOCK_MONOTONIC, &end);
            
            elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
            elapsed_sec = (double)elapsed_ns / 1000000000.0;
            res_str = mpz_get_str(NULL, 10, res_linear);
            fprintf(jsonl_file, "{\"algoritmo\": \"Linear\", \"n\": %d, \"iteracao\": %d, \"resultado\": \"%s\", \"tempo_ns\": %lld, \"tempo_segundos\": %.9f}\n", n, iter, res_str, elapsed_ns, elapsed_sec);
            free(res_str);

            // --- Teste Fast Doubling ---
            clock_gettime(CLOCK_MONOTONIC, &start);
            fib_fast_doubling(res_fast, n);
            clock_gettime(CLOCK_MONOTONIC, &end);
            
            elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
            elapsed_sec = (double)elapsed_ns / 1000000000.0;
            res_str = mpz_get_str(NULL, 10, res_fast);
            fprintf(jsonl_file, "{\"algoritmo\": \"Fast Doubling\", \"n\": %d, \"iteracao\": %d, \"resultado\": \"%s\", \"tempo_ns\": %lld, \"tempo_segundos\": %.9f}\n", n, iter, res_str, elapsed_ns, elapsed_sec);
            free(res_str);

            mpz_clears(res_linear, res_fast, NULL);
        }
    }
    logger("INFO", "Concluido. Salvo em resultados_otimizados.jsonl");
    fclose(jsonl_file); close_logger();
    return EXIT_SUCCESS;
}