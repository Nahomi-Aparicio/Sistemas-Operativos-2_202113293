#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

#define SYS_MY_ENCRYPT 551  // El número de syscall que registraste en syscall_64.tbl

void print_usage(const char *program) {
    printf("Usage: %s -p <input_file> -o <output_file> -j <num_threads_file> -k <key_file>\n", program);
}

int read_num_threads_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file for reading number of threads");
        return -1;
    }

    int num_threads;
    if (fscanf(file, "%d", &num_threads) != 1) {
        perror("Error reading number of threads from file");
        fclose(file);
        return -1;
    }

    fclose(file);
    return num_threads;
}

int main(int argc, char *argv[]) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *key_file = NULL;
    char *num_threads_file = NULL;
    int num_threads = 0;
    int opt;

    // Parsear los argumentos de línea de comandos
    while ((opt = getopt(argc, argv, "p:o:j:k:")) != -1) {
        switch (opt) {
            case 'p':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'j':
                num_threads_file = optarg;  // Ruta al archivo que contiene el número de hilos
                break;
            case 'k':
                key_file = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Verificar que todos los argumentos sean proporcionados
    if (!input_file || !output_file || !num_threads_file || !key_file) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Leer el número de hilos desde el archivo
    num_threads = read_num_threads_from_file(num_threads_file);
    if (num_threads <= 0) {
        fprintf(stderr, "Invalid number of threads: %d\n", num_threads);
        return EXIT_FAILURE;
    }

    // Mostrar los valores de los parámetros ingresados
    printf("Input file: %s\n", input_file);
    printf("Output file: %s\n", output_file);
    printf("Key file: %s\n", key_file);
    printf("Number of threads: %d\n", num_threads);

    // Invocar la syscall de encriptación
    long result = syscall(SYS_MY_ENCRYPT, input_file, output_file, key_file, num_threads);
    if (result == -1) {
        perror("Error executing syscall");
        return EXIT_FAILURE;
    }

    printf("File encrypted successfully.\n");
    return EXIT_SUCCESS;
}

