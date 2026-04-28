#define _POSIX_C_SOURCE 199309L // Necessário para usar clock_gettime
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>
#include <stdarg.h>

// ==========================================
// 1. SISTEMA DE LOGGING CUSTOMIZADO
// ==========================================
FILE *log_file = NULL;

void init_logger() {
    log_file = fopen("benchmark.log", "w");
    if (!log_file) {
        perror("Falha ao criar o arquivo de log");
        exit(EXIT_FAILURE);
    }
}

void close_logger() {
    if (log_file) fclose(log_file);
}

void logger(const char *level, const char *format, ...) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);

    va_list args_console, args_file;
    va_start(args_console, format);
    va_copy(args_file, args_console);

    printf("[%s] [%s] ", time_str, level);
    vprintf(format, args_console);
    printf("\n");

    if (log_file) {
        fprintf(log_file, "[%s] [%s] ", time_str, level);
        vfprintf(log_file, format, args_file);
        fprintf(log_file, "\n");
        fflush(log_file);
    }

    va_end(args_console);
    va_end(args_file);
}

// ==========================================
// 2. FUNÇÃO FIBONACCI (GMP RECURSIVA)
// ==========================================
void fib_gmp_recursive(mpz_t result, int n) {
    if (n == 0) {
        mpz_set_ui(result, 0);
        return;
    }
    if (n == 1) {
        mpz_set_ui(result, 1);
        return;
    }

    mpz_t r1, r2;
    mpz_init(r1);
    mpz_init(r2);

    fib_gmp_recursive(r1, n - 1);
    fib_gmp_recursive(r2, n - 2);

    mpz_add(result, r1, r2);

    mpz_clear(r1);
    mpz_clear(r2);
}

// ==========================================
// 3. MOTOR DO BENCHMARK
// ==========================================
int main() {
    init_logger();
    logger("INFO", "Iniciando Benchmark: Fibonacci GMP Recursivo");

    FILE *jsonl_file = fopen("resultados_benchmark.jsonl", "w");
    if (!jsonl_file) {
        logger("ERROR", "Nao foi possivel criar o arquivo JSONL.");
        close_logger();
        return EXIT_FAILURE;
    }
    int tamanhos[] = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67};
    int total_iteracoes = 13;

    logger("INFO", "Configuracao: 15 tamanhos de N, 13 iteracoes cada.");

    for (int i = 0; i < 15; i++) {
        int n = tamanhos[i];
        logger("INFO", "Iniciando bateria para N = %d", n);

        for (int iter = 1; iter <= total_iteracoes; iter++) {
            mpz_t resultado;
            mpz_init(resultado);

            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            fib_gmp_recursive(resultado, n);
            clock_gettime(CLOCK_MONOTONIC, &end);

            long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
            double elapsed_sec = (double)elapsed_ns / 1000000000.0;

            char *res_str = mpz_get_str(NULL, 10, resultado);

            fprintf(jsonl_file, "{\"n\": %d, \"iteracao\": %d, \"resultado\": \"%s\", \"tempo_ns\": %lld, \"tempo_segundos\": %.6f}\n", 
                    n, iter, res_str, elapsed_ns, elapsed_sec);
            logger("DEBUG", "N=%d | Iter=%d/%d | Tempo=%.6fs", n, iter, total_iteracoes, elapsed_sec);
            
            free(res_str);
            mpz_clear(resultado);
        }
    }

    logger("INFO", "Benchmark concluido com sucesso.");
    logger("INFO", "Resultados salvos em 'resultados_benchmark.jsonl'");
    logger("INFO", "Logs salvos em 'benchmark.log'");

    fclose(jsonl_file);
    close_logger();

    return EXIT_SUCCESS;
}