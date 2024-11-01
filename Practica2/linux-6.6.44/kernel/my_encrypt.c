// kernel/my_encrypt.c
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>

typedef struct {
    unsigned char *buffer;
    size_t data_size;
    unsigned char *encryption_key;
    size_t key_length;
    size_t start_idx;
    size_t end_idx;
} DataFragment;

struct task_params {
    DataFragment data_fragment;
    struct completion completed_event;
};

// Función que realiza la operación XOR en una porción de los datos
int perform_xor_operation(void *arg) {
    struct task_params *params = (struct task_params *)arg;
    DataFragment *fragment = &params->data_fragment;
    size_t i;

    printk(KERN_INFO "Thread iniciado: start_idx=%zu, end_idx=%zu\n", fragment->start_idx, fragment->end_idx);

    for (i = fragment->start_idx; i < fragment->end_idx; i++) {
        fragment->buffer[i] ^= fragment->encryption_key[i % fragment->key_length];
    }

    printk(KERN_INFO "Thread finalizado: start_idx=%zu, end_idx=%zu\n", fragment->start_idx, fragment->end_idx);
    complete(&params->completed_event);
    return 0;
}

// Función que maneja el procesamiento del archivo
int handle_file_encryption(const char *input_filepath, const char *output_filepath, const char *key_filepath, int thread_count) {
    struct file *input_file, *output_file, *key_file;
    loff_t in_offset = 0, out_offset = 0, key_offset = 0;
    unsigned char *encryption_key, *file_buffer;
    size_t file_size, key_length;
    struct task_params *task_list;
    struct task_struct **thread_list;
    DataFragment *fragment_list;
    size_t fragment_size, extra_bytes;
    int i, ret_val = 0;

    printk(KERN_INFO "Intentando abrir los archivos\n");

    input_file = filp_open(input_filepath, O_RDONLY, 0);
    output_file = filp_open(output_filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    key_file = filp_open(key_filepath, O_RDONLY, 0);

    if (IS_ERR(input_file)) {
        ret_val = PTR_ERR(input_file);
        printk(KERN_ERR "Error al abrir el archivo de entrada: %d\n", ret_val);
        goto exit;
    }
    printk(KERN_INFO "Archivo de entrada abierto correctamente\n");

    if (IS_ERR(output_file)) {
        ret_val = PTR_ERR(output_file);
        printk(KERN_ERR "Error al abrir el archivo de salida: %d\n", ret_val);
        goto close_input_file;
    }
    printk(KERN_INFO "Archivo de salida abierto correctamente\n");

    if (IS_ERR(key_file)) {
        ret_val = PTR_ERR(key_file);
        printk(KERN_ERR "Error al abrir el archivo de clave: %d\n", ret_val);
        goto close_output_file;
    }
    printk(KERN_INFO "Archivo de clave abierto correctamente\n");

    key_length = i_size_read(file_inode(key_file));
    if (key_length <= 0) {
        ret_val = -EINVAL;
        printk(KERN_ERR "Error: tamaño de la clave inválido (%zu)\n", key_length);
        goto close_key_file;
    }
    printk(KERN_INFO "Tamaño de la clave: %zu\n", key_length);

    encryption_key = kmalloc(key_length, GFP_KERNEL);
    if (!encryption_key) {
        ret_val = -ENOMEM;
        printk(KERN_ERR "Error al asignar memoria para la clave\n");
        goto close_key_file;
    }
    printk(KERN_INFO "Memoria para clave asignada correctamente\n");

    ret_val = kernel_read(key_file, encryption_key, key_length, &key_offset);
    if (ret_val < 0) {
        printk(KERN_ERR "Error al leer el archivo de clave: %d\n", ret_val);
        goto free_encryption_key;
    }
    printk(KERN_INFO "Clave leída correctamente\n");

    file_size = i_size_read(file_inode(input_file));
    if (file_size <= 0) {
        ret_val = -EINVAL;
        printk(KERN_ERR "Error: tamaño del archivo de entrada no válido (%zu)\n", file_size);
        goto free_encryption_key;
    }
    printk(KERN_INFO "Tamaño del archivo de entrada: %zu\n", file_size);

    file_buffer = kmalloc(file_size, GFP_KERNEL);
    if (!file_buffer) {
        ret_val = -ENOMEM;
        printk(KERN_ERR "Error al asignar memoria para los datos del archivo\n");
        goto free_encryption_key;
    }
    printk(KERN_INFO "Memoria para datos del archivo asignada correctamente\n");

    ret_val = kernel_read(input_file, file_buffer, file_size, &in_offset);
    if (ret_val < 0) {
        printk(KERN_ERR "Error al leer el archivo de entrada: %d\n", ret_val);
        goto free_file_buffer;
    }
    printk(KERN_INFO "Archivo de entrada leído correctamente\n");

    thread_list = kmalloc(sizeof(struct task_struct *) * thread_count, GFP_KERNEL);
    task_list = kmalloc(sizeof(struct task_params) * thread_count, GFP_KERNEL);
    fragment_list = kmalloc(sizeof(DataFragment) * thread_count, GFP_KERNEL);

    if (!thread_list || !task_list || !fragment_list) {
        ret_val = -ENOMEM;
        printk(KERN_ERR "Error al asignar memoria para hilos, tareas o fragmentos\n");
        goto free_file_buffer;
    }
    printk(KERN_INFO "Memoria para hilos, tareas y fragmentos asignada correctamente\n");

    fragment_size = file_size / thread_count;
    extra_bytes = file_size % thread_count;

    for (i = 0; i < thread_count; i++) {
        fragment_list[i].buffer = file_buffer;
        fragment_list[i].data_size = file_size;
        fragment_list[i].encryption_key = encryption_key;
        fragment_list[i].key_length = key_length;
        fragment_list[i].start_idx = i * fragment_size;
        fragment_list[i].end_idx = (i == thread_count - 1) ? (i + 1) * fragment_size + extra_bytes : (i + 1) * fragment_size;

        printk(KERN_INFO "Creando thread %d: start_idx=%zu, end_idx=%zu\n", i, fragment_list[i].start_idx, fragment_list[i].end_idx);

        task_list[i].data_fragment = fragment_list[i];
        init_completion(&task_list[i].completed_event);

        thread_list[i] = kthread_run(perform_xor_operation, &task_list[i], "xor_thread_%d", i);
        if (IS_ERR(thread_list[i])) {
            ret_val = PTR_ERR(thread_list[i]);
            printk(KERN_ERR "Error al crear el thread %d: %d\n", i, ret_val);
            goto free_all_resources;
        }
    }

    // Espera a que todos los hilos terminen
    for (i = 0; i < thread_count; i++) {
        printk(KERN_INFO "Esperando a que el thread %d termine\n", i);
        wait_for_completion(&task_list[i].completed_event);
    }

    ret_val = kernel_write(output_file, file_buffer, file_size, &out_offset);
    if (ret_val < 0) {
        printk(KERN_ERR "Error al escribir en el archivo de salida: %d\n", ret_val);
    } else {
        printk(KERN_INFO "Archivo de salida escrito exitosamente\n");
    }

free_all_resources:
    kfree(thread_list);
    kfree(task_list);
    kfree(fragment_list);

free_file_buffer:
    kfree(file_buffer);

free_encryption_key:
    kfree(encryption_key);

close_key_file:
    filp_close(key_file, NULL);

close_output_file:
    filp_close(output_file, NULL);

close_input_file:
    filp_close(input_file, NULL);

exit:
    return ret_val;
}

SYSCALL_DEFINE4(my_encrypt, const char __user *, input_filepath, const char __user *, output_filepath, const char __user *, key_filepath, int, thread_count) {
    char *k_input_filepath, *k_output_filepath, *k_key_filepath;
    int ret_val;

    printk(KERN_INFO "Entrando en la syscall my_encrypt\n");

    k_input_filepath = strndup_user(input_filepath, PATH_MAX);
    k_output_filepath = strndup_user(output_filepath, PATH_MAX);
    k_key_filepath = strndup_user(key_filepath, PATH_MAX);

    if (IS_ERR(k_input_filepath) || IS_ERR(k_output_filepath) || IS_ERR(k_key_filepath)) {
        ret_val = -EFAULT;
        printk(KERN_ERR "Error al copiar rutas desde el espacio de usuario\n");
        goto free_memory;
    }

    printk(KERN_INFO "Rutas copiadas: input=%s, output=%s, key=%s\n", k_input_filepath, k_output_filepath, k_key_filepath);

    ret_val = handle_file_encryption(k_input_filepath, k_output_filepath, k_key_filepath, thread_count);

free_memory:
    kfree(k_input_filepath);
    kfree(k_output_filepath);
    kfree(k_key_filepath);

    return ret_val;
}
