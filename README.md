# Fibonacci Benchmark

Este projeto analisa a eficiência de diferentes algoritmos para o cálculo da sequência de Fibonacci (Recursivo, Linear e Fast Doubling), comparando o crescimento do tempo de execução em relação ao termo $n$ através de visualizações geradas com Pandas e Matplotlib no arquivo `Graficos.ipynb`.

## Como Compilar e Rodar

```bash
# Compilar a versão básica (Recursiva)
gcc main_basic.c -o benchmark_basic

# Compilar a versão otimizada (Linear e Fast Doubling)
gcc main_optimized.c -o benchmark_optimized

# Executar (exemplo)
./benchmark_optimized
```
