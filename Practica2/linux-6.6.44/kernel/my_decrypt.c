// kernel/my_decrypt.c
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>

typedef struct {
    unsigned char *data;         // Buffer de datos a desencriptar
    size_t size;                 // Tamaño total de los datos
    unsigned char *key;          // Clave de desencriptado
    size_t key_size;             // Tamaño de la clave
    size_t start;                // Índice inicial para el hilo
    size_t end;                  // Índice final para el hilo
} DataChunk;

struct thread_args {
    DataChunk chunk;             // Fragmento de datos para el hilo
    struct completion done;      // Sincronización del hilo
};

// Función que realiza la operación XOR para desencriptar
int xor_decrypt_worker(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    DataChunk *chunk = &args->chunk;
    size_t i;

    printk(KERN_INFO "Hilo de desencriptado iniciado: start=%zu, end=%zu\n", chunk->start, chunk->end);

    for (i = chunk->start; i < chunk->end; i++) {
        chunk->data[i] ^= chunk->key[i % chunk->key_size];  // Desencriptar usando XOR
    }

    printk(KERN_INFO "Hilo de desencriptado completado: start=%zu, end=%zu\n", chunk->start, chunk->end);
    complete(&args->done);  // Señalizar que el hilo ha terminado
    return 0;
}

// Función que maneja el proceso de desencriptado del archivo
int decrypt_file_process(const char *input_file, const char *output_file, const char *key_file, int num_threads) {
    struct file *infile, *outfile, *keyfile;
    loff_t in_offset = 0, out_offset = 0, key_offset = 0;
    unsigned char *key, *data;
    size_t input_size, key_size;
    struct thread_args *args_list;
    struct task_struct **threads;
    DataChunk *chunks;
    size_t chunk_size, remainder;
    int i, ret = 0;

    printk(KERN_INFO "Abriendo archivos para desencriptado: input=%s, output=%s, key=%s\n", input_file, output_file, key_file);

    infile = filp_open(input_file, O_RDONLY, 0);
    outfile = filp_open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    keyfile = filp_open(key_file, O_RDONLY, 0);

    if (IS_ERR(infile)) {
        ret = PTR_ERR(infile);
        printk(KERN_ERR "Error al abrir archivo de entrada: %d\n", ret);
        goto out;
    }
    printk(KERN_INFO "Archivo de entrada abierto correctamente\n");

    if (IS_ERR(outfile)) {
        ret = PTR_ERR(outfile);
        printk(KERN_ERR "Error al abrir archivo de salida: %d\n", ret);
        goto close_infile;
    }
    printk(KERN_INFO "Archivo de salida abierto correctamente\n");

    if (IS_ERR(keyfile)) {
        ret = PTR_ERR(keyfile);
        printk(KERN_ERR "Error al abrir archivo de clave: %d\n", ret);
        goto close_outfile;
    }
    printk(KERN_INFO "Archivo de clave abierto correctamente\n");

    key_size = i_size_read(file_inode(keyfile));
    if (key_size <= 0) {
        ret = -EINVAL;
        printk(KERN_ERR "Tamaño de clave inválido: %zu\n", key_size);
        goto close_keyfile;
    }

    key = kmalloc(key_size, GFP_KERNEL);
    if (!key) {
        ret = -ENOMEM;
        printk(KERN_ERR "Error al asignar memoria para clave\n");
        goto close_keyfile;
    }

    ret = kernel_read(keyfile, key, key_size, &key_offset);
    if (ret < 0) {
        printk(KERN_ERR "Error al leer clave: %d\n", ret);
        goto free_key;
    }
    printk(KERN_INFO "Clave leída correctamente\n");

    input_size = i_size_read(file_inode(infile));
    if (input_size <= 0) {
        ret = -EINVAL;
        printk(KERN_ERR "Tamaño de archivo de entrada no válido: %zu\n", input_size);
        goto free_key;
    }
    printk(KERN_INFO "Tamaño del archivo de entrada: %zu\n", input_size);

    data = kmalloc(input_size, GFP_KERNEL);
    if (!data) {
        ret = -ENOMEM;
        printk(KERN_ERR "Error al asignar memoria para datos\n");
        goto free_key;
    }

    ret = kernel_read(infile, data, input_size, &in_offset);
    if (ret < 0) {
        printk(KERN_ERR "Error al leer archivo de entrada: %d\n", ret);
        goto free_data;
    }
    printk(KERN_INFO "Archivo de entrada leído correctamente\n");

    // Asignar memoria para hilos, argumentos y fragmentos
    threads = kmalloc(sizeof(struct task_struct *) * num_threads, GFP_KERNEL);
    args_list = kmalloc(sizeof(struct thread_args) * num_threads, GFP_KERNEL);
    chunks = kmalloc(sizeof(DataChunk) * num_threads, GFP_KERNEL);

    if (!threads || !args_list || !chunks) {
        ret = -ENOMEM;
        printk(KERN_ERR "Error al asignar memoria para hilos, argumentos o fragmentos\n");
        goto free_data;
    }

    chunk_size = input_size / num_threads;
    remainder = input_size % num_threads;

    for (i = 0; i < num_threads; i++) {
        chunks[i].data = data;
        chunks[i].size = input_size;
        chunks[i].key = key;
        chunks[i].key_size = key_size;
        chunks[i].start = i * chunk_size;
        chunks[i].end = (i == num_threads - 1) ? (i + 1) * chunk_size + remainder : (i + 1) * chunk_size;

        printk(KERN_INFO "Creando hilo de desencriptado %d: start=%zu, end=%zu\n", i, chunks[i].start, chunks[i].end);

        args_list[i].chunk = chunks[i];
        init_completion(&args_list[i].done);

        threads[i] = kthread_run(xor_decrypt_worker, &args_list[i], "decrypt_thread_%d", i);
        if (IS_ERR(threads[i])) {
            ret = PTR_ERR(threads[i]);
            printk(KERN_ERR "Error al crear hilo de desencriptado %d: %d\n", i, ret);
            goto free_all;
        }
    }

    // Esperar a que todos los hilos terminen
    for (i = 0; i < num_threads; i++) {
        printk(KERN_INFO "Esperando que el hilo de desencriptado %d termine\n", i);
        wait_for_completion(&args_list[i].done);
    }

    ret = kernel_write(outfile, data, input_size, &out_offset);
    if (ret < 0) {
        printk(KERN_ERR "Error al escribir archivo de salida: %d\n", ret);
    } else {
        printk(KERN_INFO "Archivo de salida escrito exitosamente\n");
    }

free_all:
    kfree(threads);
    kfree(args_list);
    kfree(chunks);

free_data:
    kfree(data);

free_key:
    kfree(key);

close_keyfile:
    filp_close(keyfile, NULL);

close_outfile:
    filp_close(outfile, NULL);

close_infile:
    filp_close(infile, NULL);

out:
    return ret;
}

// Definición del syscall para desencriptar
SYSCALL_DEFINE4(my_decrypt, const char __user *, input_path, const char __user *, output_path, const char __user *, key_path, int, num_threads) {
    char *k_input_path, *k_output_path, *k_key_path;
    int ret;

    printk(KERN_INFO "Entrando al syscall my_decrypt\n");

    k_input_path = strndup_user(input_path, PATH_MAX);
    k_output_path = strndup_user(output_path, PATH_MAX);
    k_key_path = strndup_user(key_path, PATH_MAX);

    if (IS_ERR(k_input_path) || IS_ERR(k_output_path) || IS_ERR(k_key_path)) {
        ret = -EFAULT;
        printk(KERN_ERR "Error al copiar rutas desde el espacio de usuario\n");
        goto out_free;
    }

    printk(KERN_INFO "Rutas copiadas del espacio de usuario: input=%s, output=%s, key=%s\n", k_input_path, k_output_path, k_key_path);

    ret = decrypt_file_process(k_input_path, k_output_path, k_key_path, num_threads);

out_free:
    kfree(k_input_path);
    kfree(k_output_path);
    kfree(k_key_path);

    return ret;
}
